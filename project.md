# Roadmap status

Legend: ✅ done · 🟡 partial · ⬜ not started

| Version  | Goal                    | Why it matters              | Status                       |
| -------- | ----------------------- | --------------------------- | ---------------------------- |
| **v0.2** | Refactor architecture   | Clean foundation            | ✅ Done                       |
| **v0.3** | Database + Value object | Supports future data types  | 🟡 Partial (Database only)   |
| **v0.4** | TTL                     | First real Redis feature    | ✅ Done                       |
| **v0.5** | AOF persistence         | Learn durability            | 🟡 Architecture prepared     |
| **v0.6** | RESP protocol           | Compatible with `redis-cli` | ⬜ Not started               |
| **v0.7** | Benchmarks              | Measure performance         | ⬜ Not started               |
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

- **v0.5 — Architecture prepared, AOF not implemented.** Persistence is now a
  strategy behind the `PersistenceManager` interface, injected into
  `StorageEngine`. `SnapshotPersistence` (full-file rewrite) is the current
  implementation and survives restarts. AOF itself is not written yet; the seam
  for adding it is documented in `docs/ARCHITECTURE.md`.

## Summary

- **Fully done:** v0.2, v0.4
- **Partial / prepared:** v0.3 (Database, no Value object), v0.5 (pluggable
  persistence + snapshot, no AOF)
- **Not started:** v0.6 – v1.2

Roughly **2 of 11 versions fully complete**, with the foundations for v0.3 and
v0.5 already in place.
