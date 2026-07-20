# vibes-ds-client

A small, dependency-free TypeScript client for
[vibes-ds-server](../README.md) — a Redis-like key-value store served over HTTP.

- Typed methods for every server command (strings, numeric, expiration,
  keyspace, server, persistence).
- Works in Node 18+ and modern browsers (uses the global `fetch`).
- Ships ESM, CommonJS, and type declarations.
- Integration tests run against a **real server in Docker** — no mocking.

## Install

```bash
npm install vibes-ds-client
```

## Usage

Commands are organized into resource groups that mirror the server's command
categories: `strings`, `numeric`, `expiration`, `keyspace`, `server`,
`persistence`.

```ts
import { VibesClient } from "vibes-ds-client";

const client = new VibesClient({ host: "localhost", port: 8080 });
// or: new VibesClient({ baseUrl: "http://localhost:8080" });

await client.strings.set("name", "Vaibu");
await client.strings.get("name"); // "Vaibu"
await client.strings.get("missing"); // null

await client.numeric.incr("counter"); // 1
await client.expiration.expire("counter", 60); // true
await client.expiration.ttl("counter"); // ~60

await client.strings.mset({ a: "1", b: "2" });
await client.strings.mget("a", "b", "ghost"); // ["1", "2", null]

await client.keyspace.keys("user:*"); // string[]
await client.server.info(); // grouped stats text
await client.persistence.save();

// Escape hatch for anything without a dedicated method:
await client.command("SET", "k", "v"); // "OK"
```

Resource modules can also be used à la carte with a shared `Transport`:

```ts
import { Transport, StringCommands } from "vibes-ds-client";

const transport = new Transport({ baseUrl: "http://localhost:8080" });
const strings = new StringCommands(transport);
await strings.set("k", "v");
```

### Errors

| Error                  | When                                                    |
| ---------------------- | ------------------------------------------------------- |
| `VibesArgumentError`   | An argument fails client-side validation (see below).   |
| `VibesCommandError`    | The server rejects the command (HTTP 400).              |
| `VibesConnectionError` | The server can't be reached (network/timeout).          |
| `VibesError`           | Base class for all of the above.                        |

```ts
import { VibesCommandError } from "vibes-ds-client";

try {
  await client.incr("not-a-number");
} catch (err) {
  if (err instanceof VibesCommandError) {
    console.error(err.command, err.message);
  }
}
```

## Limitation: no whitespace in keys/values

The server tokenizes commands on whitespace and has no quoting, so keys and
values must be **non-empty** and contain **no whitespace**. The client validates
this before sending and throws `VibesArgumentError` with a clear message.

## API

| Group                  | Methods                                                                          |
| ---------------------- | ------------------------------------------------------------------------------- |
| `client.strings`       | `set` `get` `del` `exists` `mset` `mget` `append` `strlen` `getset` `setnx` `setex` `getdel` |
| `client.numeric`       | `incr` `incrby` `decr` `decrby`                                                  |
| `client.expiration`    | `expire` `pexpire` `ttl` `pttl` `persist`                                        |
| `client.keyspace`      | `keys` `scan` `type` `rename` `randomKey`                                        |
| `client.server`        | `ping` `echo` `info` `dbsize` `flushdb` `flushall`                               |
| `client.persistence`   | `save` `bgsave` `lastsave`                                                       |
| `client`               | `command(...args)` — raw escape hatch                                            |

## Development

```bash
npm install
npm run build             # bundle ESM + CJS + d.ts into dist/ (tsup)
npm run typecheck         # tsc --noEmit
npm run test:unit         # pure unit tests (fast, no Docker)
npm run test:integration  # integration tests (requires Docker)
npm test                  # unit + integration
```

Unit tests cover the pure encoding/parsing helpers and client-side validation
(no server, no mocking). Integration tests use
[`testcontainers`](https://testcontainers.com) to build the server image from
the repository `Dockerfile`, start **one** container shared by all integration
files, and exercise the client against the live HTTP endpoint. **Docker must be
running** for `test:integration`.

## Benchmark

```bash
npm run bench     # requires Docker
```

Drives the real server (in Docker) through the client and reports SET/GET
throughput and latency percentiles **once per persistence strategy**, making the
snapshot-vs-AOF write trade-off visible (snapshot rewrites the whole file per
write; AOF appends one line). Tunables via env: `BENCH_KEYS`,
`BENCH_CONCURRENCY`, `BENCH_MODES` (e.g. `BENCH_MODES=snapshot,aof`).
