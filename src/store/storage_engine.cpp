#include "store/storage_engine.h"

#include "observability/logger.h"
#include "persistence/snapshot_persistence.h"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace
{
// Where the default snapshot strategy stores its file.
const char *const kDefaultSnapshotPath = "cache/key-value.db";
} // namespace

StorageEngine::StorageEngine()
    : StorageEngine(std::make_unique<SnapshotPersistence>(kDefaultSnapshotPath))
{
}

StorageEngine::StorageEngine(std::unique_ptr<PersistenceManager> persistence)
    : persistence_(std::move(persistence))
{
    restore();
    last_save_ = std::chrono::system_clock::now();
    start_sweeper();
}

StorageEngine::~StorageEngine()
{
    {
        std::lock_guard<std::mutex> lock(sweeper_mutex_);
        running_ = false;
    }
    sweeper_cv_.notify_all();
    if (sweeper_.joinable())
    {
        sweeper_.join();
    }
}

// ---- persistence coordination --------------------------------------------

void StorageEngine::restore()
{
    std::size_t count = 0;
    for (const auto &record : persistence_->load())
    {
        db_.load_raw(record.key, record.value);
        ++count;
    }
    Logger::info("persistence", "restored " + std::to_string(count) + " key(s) from disk");
}

void StorageEngine::persist_locked()
{
    std::vector<Record> records;
    for (const auto &[key, value] : db_.snapshot())
    {
        records.push_back({key, value});
    }
    persistence_->save(records);
    stats_.record_persistence();
    Logger::debug("persistence", "saved " + std::to_string(records.size()) + " key(s)");
}

void StorageEngine::finish_operation()
{
    // Classify the just-completed operation: it is a write if it changed the
    // data (which also triggers persistence), otherwise a read.
    if (db_.take_dirty())
    {
        persist_locked();
        stats_.record_write();
    }
    else
    {
        stats_.record_read();
    }
}

void StorageEngine::save()
{
    std::lock_guard<std::mutex> lock(mutex_);
    persist_locked();
    last_save_ = std::chrono::system_clock::now();
    Logger::info("persistence", "explicit save completed");
}

long long StorageEngine::last_save_epoch() const
{
    return std::chrono::duration_cast<std::chrono::seconds>(
               last_save_.time_since_epoch())
        .count();
}

// ---- observability -------------------------------------------------------

void StorageEngine::record_command(std::chrono::microseconds latency)
{
    stats_.record_command(latency);
}

InfoSnapshot StorageEngine::info_snapshot()
{
    std::lock_guard<std::mutex> lock(mutex_);

    InfoSnapshot snap;
    // Server
    snap.uptime_seconds = stats_.uptime_seconds();
    snap.start_epoch = stats_.start_epoch();
    // Stats owned by the engine
    snap.total_commands = stats_.commands();
    snap.total_reads = stats_.reads();
    snap.total_writes = stats_.writes();
    snap.persistence_ops = stats_.persistence_ops();
    snap.expired_sweeper = stats_.sweeper_expired();
    snap.avg_command_latency_us = stats_.average_latency_us();
    snap.last_persistence_epoch = stats_.last_persistence_epoch();
    // Memory + keyspace (from Database; size() also purges, so read counters after)
    snap.key_count = db_.size();
    snap.dataset_bytes = db_.approx_bytes();
    const auto keyspace = db_.keyspace_stats();
    snap.cache_hits = keyspace.hits;
    snap.cache_misses = keyspace.misses;
    snap.expired_lazy = keyspace.lazy_expired;
    return snap;
}

// ---- active expiration ---------------------------------------------------

void StorageEngine::start_sweeper()
{
    sweeper_ = std::thread(&StorageEngine::sweep_loop, this);
}

void StorageEngine::sweep_loop()
{
    using namespace std::chrono_literals;
    std::unique_lock<std::mutex> wait_lock(sweeper_mutex_);
    while (running_)
    {
        // Wake up periodically, or immediately when shutting down.
        sweeper_cv_.wait_for(wait_lock, 1s, [this] { return !running_.load(); });
        if (!running_)
        {
            break;
        }

        std::size_t removed = 0;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            removed = db_.purge_expired();
        }
        if (removed > 0)
        {
            stats_.record_sweeper_expired(removed);
            Logger::info("ttl",
                         "sweeper removed " + std::to_string(removed) + " expired key(s)");
        }
    }
}
