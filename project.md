# Roadmap status

Legend: ✅ done · 🟡 partial · ⬜ not started

| Version  | Goal                    | Why it matters              | Status                       |
| -------- | ----------------------- | --------------------------- | ---------------------------- |
| **v0.2** | Refactor architecture   | Clean foundation            | ✅ Done                       |
| **v0.3** | Database + Value object | Supports future data types  | 🟡 Partial (Database only)   |
| **v0.4** | TTL                     | First real Redis feature    | ✅ Done                       |
| **v0.5** | AOF persistence         | Learn durability            | ✅ Done                       |
| **v0.6** | RESP protocol           | Compatible with `redis-cli` | ⬜ Not started               |
| **v0.7** | Benchmarks              | Measure performance         | ✅ Done                       |
| **v0.8** | Hashes                  | New data structures         | ⬜ Not started               |
| **v0.9** | Lists                   | More complex storage        | ⬜ Not started               |
| **v1.0** | Transactions            | Atomic operations           | ⬜ Not started               |
| **v1.1** | Pub/Sub                 | Real-time messaging         | ⬜ Not started               |
| **v1.2** | Replication             | Distributed systems         | ⬜ Not started               |

## Out-of-roadmap tooling (done)

These were built to support the roadmap rather than being numbered milestones:

- **Observability & developer tooling — ✅ Done.** Centralized `Logger`
  (leveled, thread-safe), a `Statistics` module owned by `StorageEngine`
  (commands, reads/writes, cache hits/misses, lazy/sweeper expirations,
  persistence ops, key count, uptime), per-command latency via `std::chrono`,
  and a Redis-like `INFO` command grouped into `# Server / # Memory / # Stats /
  # Persistence / # Keyspace`. This also lays the groundwork for **v0.7
  (Benchmarks)**, since latency and throughput are now measurable in-process.
- **TypeScript client library — ✅ Done.** A publishable npm package
  (`client/`, `vibes-ds-client`) with typed methods for every command and
  integration tests that run against a real server spun up in Docker (no
  mocking). See `client/README.md`.

## Notes on partial / prepared items

- **v0.2 — Done.** Layered modules with clear boundaries: `net/`, `http/`,
  `server/`, `app/controllers/`, `redis/` (`CommandExecutor` + `commands/*`),
  `store/` (`StorageEngine` + `Database`), `persistence/`. See
  `docs/ARCHITECTURE.md`.

- **v0.3 — Partial.** The `Database` (pure in-memory keyspace) is complete and
  cleanly separated from threading/persistence. The polymorphic **Value object**
  is intentionally deferred: values are still `std::string` by design, to be
  introduced only when Hashes (v0.8) actually need it.

- **v0.4 — Done.** `EXPIRE`, `PEXPIRE`, `TTL`, `PTTL`, `PERSIST`, `SETEX`, with
  lazy expiry on access plus an active background sweeper.

- **v0.5 — Done.** Persistence is a strategy behind the `PersistenceManager`
  interface (`load` / `record` / `checkpoint`), injected into `StorageEngine`
  and selected at startup via `VIBES_PERSISTENCE=snapshot|aof`.
  `SnapshotPersistence` rewrites the full dataset; `AofPersistence` appends each
  mutation to a log (fsync policy), replays it on startup, and compacts on
  `SAVE`/`BGSAVE` or automatically past a threshold. `Database` gained a change
  journal (`Mutation`s) that AOF logs. See `docs/ARCHITECTURE.md`.

- **v0.7 — Done.** A benchmark harness (`client/bench`, `npm run bench`) drives
  the real server in Docker through the client and reports SET/GET throughput
  and latency percentiles per persistence strategy. It confirms AOF is ~2x
  faster on writes than snapshot (append vs whole-file rewrite), with the gap
  widening as the key count grows; reads are comparable.

## Summary

- **Fully done:** v0.2, v0.4, v0.5, v0.7
- **Partial / prepared:** v0.3 (Database, no Value object — deferred until Hashes)
- **Not started:** v0.6, v0.8 – v1.2

Roughly **4 of 11 versions fully complete**, with v0.3's foundation in place.
