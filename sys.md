Phase 1: Match Redis Basics
SET
GET
DEL
EXISTS
EXPIRE
TTL
INCR
DECR
MSET
MGET

You'll immediately learn:

Hash tables
Time complexity
Expiration strategies
Phase 2: Persistence

Implement:

Snapshot (RDB-like)
Append-only log (AOF)

Now you'll understand:

Durability
Crash recovery
fsync
Trade-offs

Benchmark:

Startup time
Write latency
Phase 3: Expiration

Implement:

Lazy expiration
Active expiration

Then compare with Redis.

Phase 4: Data Structures

Support

Strings
Lists
Sets
Sorted Sets
Hashes

This is where you'll appreciate why Redis isn't just a std::unordered_map.

Phase 5: Networking

Currently you have HTTP.

Replace it with a TCP protocol.

Then implement:

RESP protocol

Now Redis clients can actually talk to your server.

Phase 6: Performance

Benchmark against Redis.

Measure:

GET QPS
SET QPS
Latency
Memory usage
Startup time

Use tools like:

redis-benchmark
wrk
hey
Phase 7: Threading

Current

1 thread

Then

Worker threads

Then

Thread pool

Measure again.

Phase 8: Replication

Leader

↓

Replica

Learn:

Log shipping
Replication lag
Failover
Phase 9: Transactions

Implement

MULTI
EXEC
WATCH
Phase 10: Pub/Sub

Implement

SUBSCRIBE
PUBLISH
Phase 11: LRU Cache

Implement

LRU
LFU
FIFO

Compare memory and hit rates.

Phase 12: Cluster (Stretch Goal)

Multiple nodes

Hash slot routing

Consistent hashing