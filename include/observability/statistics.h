#ifndef OBSERVABILITY_STATISTICS_H
#define OBSERVABILITY_STATISTICS_H

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>

// A consistent, point-in-time view of the server's counters, assembled by
// StorageEngine for the INFO command. Fields are grouped to match INFO sections.
struct InfoSnapshot
{
    // Server
    long long uptime_seconds = 0;
    long long start_epoch = 0;

    // Memory (approximate: sum of key+value bytes; not real allocator usage)
    std::size_t key_count = 0;
    std::size_t dataset_bytes = 0;

    // Stats
    std::uint64_t total_commands = 0;
    std::uint64_t total_reads = 0;
    std::uint64_t total_writes = 0;
    std::uint64_t cache_hits = 0;
    std::uint64_t cache_misses = 0;
    std::uint64_t expired_lazy = 0;
    std::uint64_t expired_sweeper = 0;
    double avg_command_latency_us = 0.0;

    // Persistence
    std::uint64_t persistence_ops = 0;
    long long last_persistence_epoch = 0;  // 0 = nothing persisted this run
};

// Thread-safe counters owned by StorageEngine.
//
// Every counter is atomic so it can be updated from any connection thread or the
// sweeper without extra locking. Statistics only holds the counters the engine
// itself observes (commands, reads/writes, persistence, sweeper expirations,
// latency, timings). Counters that only Database can see -- cache hits/misses,
// lazy expirations, key count, dataset size -- are read from Database when the
// snapshot is assembled, so Statistics has no dependency on Database.
class Statistics
{
public:
    Statistics();

    void record_command(std::chrono::microseconds latency);
    void record_read();
    void record_write();
    void record_persistence();
    void record_sweeper_expired(std::uint64_t count);

    std::uint64_t commands() const;
    std::uint64_t reads() const;
    std::uint64_t writes() const;
    std::uint64_t persistence_ops() const;
    std::uint64_t sweeper_expired() const;
    double average_latency_us() const;

    long long start_epoch() const;
    long long uptime_seconds() const;
    long long last_persistence_epoch() const;

private:
    std::atomic<std::uint64_t> commands_{0};
    std::atomic<std::uint64_t> reads_{0};
    std::atomic<std::uint64_t> writes_{0};
    std::atomic<std::uint64_t> persistence_ops_{0};
    std::atomic<std::uint64_t> sweeper_expired_{0};
    std::atomic<std::uint64_t> total_latency_us_{0};
    std::atomic<long long> last_persistence_epoch_{0};

    std::chrono::system_clock::time_point start_wall_;   // for start_epoch display
    std::chrono::steady_clock::time_point start_steady_; // for monotonic uptime
};

#endif  // OBSERVABILITY_STATISTICS_H
