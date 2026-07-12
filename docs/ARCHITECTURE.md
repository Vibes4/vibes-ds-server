# Architecture

`vibes-ds-server` is a small, Redis-like key-value store served over HTTP. It is
organized in layers, each with a single responsibility and depending only on the
layer beneath it. Networking does not know about HTTP; HTTP does not know about
commands; commands do not know about threading or disk; and the storage format
is hidden behind an interface.

```
HTTP client
     │  TCP bytes
     ▼
[ net/ ]          TcpServer            accept loop, one detached thread per conn
     │  raw request bytes
     ▼
[ server/ ]       HttpServer           read → parse → route → write (wiring root)
     │  HttpRequest
     ▼
[ http/ ]         Router               path prefix → controller handler
     │  HttpRequest
     ▼
[ app/ ]          RedisController      /redis : read `cmd`, call the executor
                  HealthController     /ping  : liveness / info text
     │  command line ("SET a 1")
     ▼
[ redis/ ]        CommandExecutor      tokenise + look up + dispatch
                  commands/*           one Command class per command
     │  Command::execute(engine, args)
     ▼
[ store/ ]        StorageEngine        thread-safe: with_lock(fn(Database&))
                  Database             pure in-memory keyspace + TTL
     │  save(records) when data changed
     ▼
[ persistence/ ]  PersistenceManager   interface
                  SnapshotPersistence  impl — rewrites cache/key-value.db
```

## Modules

| Module         | Key types                                                | Responsibility                                                                                                                                                                                                       |
| -------------- | -------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `platform/`    | `platform.h`, `close_socket()`                           | Cross-platform socket types (Winsock vs BSD). Everything else is platform-agnostic.                                                                                                                                 |
| `net/`         | `TcpServer`                                              | Owns the listening socket and the accept loop. Spawns one detached thread per connection and hands the client socket to a callback. Knows nothing about HTTP.                                                        |
| `http/`        | `HttpRequest`, `HttpResponse`, `Router`                  | Parse the request line into method/path/query (with URL-decoding), build/serialize responses, and route by path prefix to a handler.                                                                                |
| `server/`      | `HttpServer`                                             | The wiring root: reads bytes → `HttpRequest::parse` → `Router::route` → `write_response`. Owns the `Router`, the `CommandExecutor`, and the controllers, and registers each controller's routes.                     |
| `app/`         | `RedisController`, `HealthController` (`controllers/`)   | The **controllers**: thin HTTP adapters. `RedisController` handles `/redis` (pull the `cmd` param, call the executor, wrap the result). `HealthController` handles `/ping`. Each registers its own routes.            |
| `redis/`       | `CommandExecutor`, `Command`, `commands/*`               | `CommandExecutor` tokenizes a command line, looks the command up in its registry, and dispatches to a `Command`. It holds no business logic. Each command (`SET`, `INCR`, `EXPIRE`, …) is its own class.             |
| `store/`       | `StorageEngine`, `Database`                              | `Database` is the pure in-memory keyspace (a string map with per-key TTL and glob matching) — no locking, no I/O. `StorageEngine` is the thread-safe engine that owns the `Database`, the mutex, the `PersistenceManager`, and the TTL sweeper. |
| `persistence/` | `PersistenceManager`, `SnapshotPersistence`, `Record`    | Durability, behind an interface. `StorageEngine` depends only on `PersistenceManager`; `SnapshotPersistence` is the current implementation (full-file rewrite).                                                      |

## The flow of a request

Take `GET /redis?cmd=SET+name+Vaibu`:

1. **net** — `TcpServer` accepts the connection and runs `HttpServer::handle_connection` on a new thread with the client socket.
2. **server** — `handle_connection` `recv`s the bytes and calls `HttpRequest::parse`, producing `{ method: "GET", path: "/redis", query: { cmd: "SET name Vaibu" } }` (the `+` is URL-decoded to a space).
3. **http/router** — `Router::route` finds the first registered prefix that matches `/redis` and calls its handler.
4. **app (controller)** — `RedisController::handle_command` reads `request.param("cmd")` and calls `CommandExecutor::execute("SET name Vaibu")`.
5. **redis** — `CommandExecutor` tokenizes to `["SET", "name", "Vaibu"]`, upper-cases `SET`, finds `SetCommand` in its registry, and calls `execute(engine, ["name", "Vaibu"])`.
6. **redis (command)** — `SetCommand` validates argument count and runs its logic inside `engine.with_lock([&](Database& db){ db.set_value("name", "Vaibu"); })`, returning `{ok: true, "OK"}`.
7. **store** — `with_lock` runs the function under the mutex. Because `set_value` marked the keyspace dirty, `StorageEngine` builds the current dataset and calls `persistence_->save(records)`.
8. **persistence** — `SnapshotPersistence::save` rewrites `cache/key-value.db`.
9. The `CommandResult` bubbles back up: `RedisController` turns `{true, "OK"}` into `HttpResponse::ok("OK")`, and `server` serializes it back over the socket.

Reads (`GET`, `TTL`, `KEYS`, …) follow the same path but do not mark the keyspace
dirty, so step 7 skips persistence.

## Design decisions worth knowing

- **One lock, one seam.** `Database` is deliberately not thread-safe. All
  concurrency goes through `StorageEngine::with_lock`, which is the single place
  the mutex is taken. This makes read-then-write commands (`INCR`, `GETSET`,
  `APPEND`, `SETNX`) atomic without scattering locks, and keeps each command's
  logic in its own file.

- **The executor holds no business logic.** `CommandExecutor` only tokenizes the
  command line, looks up the `Command`, and runs it. What each command *means*
  lives in the command class; how it is stored lives in `StorageEngine` /
  `Database`. Controllers only adapt HTTP to/from the executor.

- **Persistence is a strategy, not a hard-coded file write.** `StorageEngine`
  depends on the `PersistenceManager` interface and receives an implementation by
  constructor injection (the default constructor installs `SnapshotPersistence`).
  Swapping or adding a strategy is a localized change behind the interface; no
  engine, command, or transport code changes.

- **Write-through with a dirty flag.** Mutating primitives on `Database` set a
  `dirty_` flag; `StorageEngine` persists only when a committed operation actually
  changed the data. TTL changes and lazy expiry do **not** dirty the keyspace,
  matching the fact that expiry is in-memory only.

- **Expiration is in-memory only.** TTLs are not written to disk, so a key
  reloaded after a restart comes back without its TTL. Expired keys are removed
  both lazily (on access, inside `Database`) and actively (by `StorageEngine`'s
  background sweeper thread).

## Extending persistence (e.g. AOF later)

The append-only-file strategy is intentionally **not** implemented yet; the
architecture is prepared for it:

1. Add `AofPersistence : public PersistenceManager` under `persistence/`.
2. If AOF needs per-mutation granularity, add an incremental hook (e.g.
   `append(op)`) to the `PersistenceManager` interface. `SnapshotPersistence`
   would ignore it and keep rewriting on `save()`; `StorageEngine` would call it
   from `commit_if_dirty`.
3. Choose the strategy at the composition root and inject it via
   `StorageEngine(std::unique_ptr<PersistenceManager>)`.

Nothing above the `store/` layer is affected by any of this.
