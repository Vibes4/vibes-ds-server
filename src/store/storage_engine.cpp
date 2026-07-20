#include "store/storage_engine.h"

#include "observability/logger.h"
#include "persistence/aof_persistence.h"
#include "persistence/snapshot_persistence.h"

#include <cstddef>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

namespace
{

// Translates Database's change journal into persistence write ops.
std::vector<WriteOp> to_write_ops(const std::vector<Database::Mutation> &mutations)
{
    std::vector<WriteOp> ops;
    ops.reserve(mutations.size());
    for (const auto &mutation : mutations)
    {
        switch (mutation.kind)
        {
        case Database::Mutation::Kind::Set:
            ops.push_back({WriteOp::Type::Set, mutation.key, mutation.value});
            break;
        case Database::Mutation::Kind::Delete:
            ops.push_back({WriteOp::Type::Delete, mutation.key, {}});
            break;
        case Database::Mutation::Kind::Clear:
            ops.push_back({WriteOp::Type::Clear, {}, {}});
            break;
        }
    }
    return ops;
}

std::string env_or(const char *name, const std::string &fallback)
{
    const char *value = std::getenv(name);
    return value != nullptr ? std::string(value) : fallback;
}

// Chooses the persistence strategy from the environment:
//   VIBES_PERSISTENCE = snapshot (default) | aof
//   VIBES_SNAPSHOT_PATH (default cache/key-value.db)
//   VIBES_AOF_PATH      (default cache/appendonly.aof)
std::unique_ptr<PersistenceManager> make_default_persistence()
{
    if (env_or("VIBES_PERSISTENCE", "snapshot") == "aof")
    {
        Logger::info("persistence", "using append-only-file (AOF) strategy");
        return std::make_unique<AofPersistence>(env_or("VIBES_AOF_PATH", "cache/appendonly.aof"));
    }
    Logger::info("persistence", "using snapshot strategy");
    return std::make_unique<SnapshotPersistence>(env_or("VIBES_SNAPSHOT_PATH", "cache/key-value.db"));
}

} // namespace

StorageEngine::StorageEngine() : StorageEngine(make_default_persistence())
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

std::vector<Record> StorageEngine::current_dataset()
{
    std::vector<Record> records;
    for (const auto &[key, value] : db_.snapshot())
    {
        records.push_back({key, value});
    }
    return records;
}

void StorageEngine::finish_operation()
{
    // Classify the just-completed operation: it is a write if it changed the
    // data (which also triggers persistence), otherwise a read. The mutation
    // journal is always drained so it never leaks into the next operation.
    const bool changed = db_.take_dirty();
    std::vector<Database::Mutation> mutations = db_.take_mutations();
    if (!changed)
    {
        stats_.record_read();
        return;
    }

    // Snapshot ignores `ops` and writes `dataset`; AOF appends `ops` and skips
    // the (lazy) dataset -- so it is never built on the append-only hot path.
    persistence_->record(to_write_ops(mutations), [this] { return current_dataset(); });
    stats_.record_persistence();
    stats_.record_write();
}

void StorageEngine::save()
{
    std::lock_guard<std::mutex> lock(mutex_);
    persistence_->checkpoint([this] { return current_dataset(); });
    stats_.record_persistence();
    last_save_ = std::chrono::system_clock::now();
    Logger::info("persistence", "explicit checkpoint completed");
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
