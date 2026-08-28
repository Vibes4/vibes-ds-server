#ifndef STORE_STORAGE_ENGINE_H
#define STORE_STORAGE_ENGINE_H

#include "observability/statistics.h"
#include "persistence/persistence_manager.h"
#include "store/database.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <type_traits>

// The thread-safe, persistent database engine: the front door to the keyspace.
//
// StorageEngine owns the single Database, the single mutex that guards it, a
// PersistenceManager (an abstraction -- it never touches files directly), the
// background TTL sweeper, and the Statistics counters.
// Command controllers never reach the Database themselves; they pass a function
// to with_lock(), which runs it under the lock and, if it changed the data,
// hands the new dataset to the persistence strategy. This gives every command
// one atomic critical section while keeping each command's logic in its own
// file, and keeps the choice of storage format entirely behind PersistenceManager.
//
// Expiration is in-memory only (not persisted). Expired keys are removed lazily
// on access (inside Database) and actively by a background sweeper thread.
class StorageEngine
{
public:
    // Uses the default persistence strategy: a snapshot file under cache/.
    StorageEngine();
    // Uses a caller-supplied persistence strategy (dependency injection).
    explicit StorageEngine(std::unique_ptr<PersistenceManager> persistence);
    ~StorageEngine();

    // Runs `fn(Database&)` under the lock and returns its result. If the call
    // modified the data, the new dataset is persisted afterwards.
    template <class Fn>
    auto with_lock(Fn &&fn)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        using Result = decltype(fn(db_));
        if constexpr (std::is_void_v<Result>)
        {
            fn(db_);
            finish_operation();
        }
        else
        {
            Result result = fn(db_);
            finish_operation();
            return result;
        }
    }

    // ---- persistence commands ----
    void save();                        // force a durable write now (SAVE/BGSAVE)
    long long last_save_epoch() const;  // unix seconds of the last save (LASTSAVE)

    // ---- observability ----
    // Records that a command finished, with how long it took (for latency stats).
    void record_command(std::chrono::microseconds latency);
    // Assembles a consistent snapshot of all counters for the INFO command.
    InfoSnapshot info_snapshot();

private:
    void restore();                    // load persisted data into db_ (constructor only)
    void finish_operation();           // count read/write and persist iff dirty; assumes mutex_ is held
    std::vector<Record> current_dataset();  // build records from db_; assumes mutex_ is held
    void start_sweeper();
    void sweep_loop();                 // background active-expiration thread body

    Database db_;
    std::unique_ptr<PersistenceManager> persistence_;
    Statistics stats_;
    std::mutex mutex_;  // guards db_ and last_save_
    std::chrono::system_clock::time_point last_save_;

    std::thread sweeper_;
    std::mutex sweeper_mutex_;
    std::condition_variable sweeper_cv_;
    std::atomic<bool> running_{true};
};

#endif  // STORE_STORAGE_ENGINE_H
