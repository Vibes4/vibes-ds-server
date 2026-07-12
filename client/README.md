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

```ts
import { VibesClient } from "vibes-ds-client";

const client = new VibesClient({ host: "localhost", port: 8080 });
// or: new VibesClient({ baseUrl: "http://localhost:8080" });

await client.set("name", "Vaibu");
await client.get("name"); // "Vaibu"
await client.get("missing"); // null

await client.incr("counter"); // 1
await client.expire("counter", 60); // true
await client.ttl("counter"); // ~60

await client.mset({ a: "1", b: "2" });
await client.mget("a", "b", "ghost"); // ["1", "2", null]

await client.keys("user:*"); // string[]
await client.info(); // grouped stats text

// Escape hatch for anything without a dedicated method:
await client.command("SET", "k", "v"); // "OK"
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

Strings: `set` `get` `del` `exists` `mset` `mget` `append` `strlen` `getset`
`setnx` `setex` `getdel`
Numeric: `incr` `incrby` `decr` `decrby`
Expiration: `expire` `pexpire` `ttl` `pttl` `persist`
Keyspace: `keys` `scan` `type` `rename` `randomKey`
Server: `ping` `echo` `info` `dbsize` `flushdb` `flushall`
Persistence: `save` `bgsave` `lastsave`
Raw: `command(...args)`

## Development

```bash
npm install
npm run build      # bundle ESM + CJS + d.ts into dist/ (tsup)
npm run typecheck  # tsc --noEmit
npm test           # integration tests (requires Docker)
```

The test suite uses [`testcontainers`](https://testcontainers.com) to build the
server image from the repository `Dockerfile`, start it, and exercise the client
against the live HTTP endpoint. **Docker must be running.**
