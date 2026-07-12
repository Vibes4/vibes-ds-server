#include "store/storage_engine.h"

#include "persistence/snapshot_persistence.h"

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
    for (const auto &record : persistence_->load())
    {
        db_.load_raw(record.key, record.value);
    }
}

void StorageEngine::persist_locked()
{
    std::vector<Record> records;
    for (const auto &[key, value] : db_.snapshot())
    {
        records.push_back({key, value});
    }
    persistence_->save(records);
}

void StorageEngine::commit_if_dirty()
{
    if (db_.take_dirty())
    {
        persist_locked();
    }
}

void StorageEngine::save()
{
    std::lock_guard<std::mutex> lock(mutex_);
    persist_locked();
    last_save_ = std::chrono::system_clock::now();
}

long long StorageEngine::last_save_epoch() const
{
    return std::chrono::duration_cast<std::chrono::seconds>(
               last_save_.time_since_epoch())
        .count();
}

std::unordered_map<std::string, std::string> StorageEngine::entries()
{
    return with_lock([](Database &db) { return db.snapshot(); });
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
        std::lock_guard<std::mutex> lock(mutex_);
        db_.purge_expired();
    }
}
