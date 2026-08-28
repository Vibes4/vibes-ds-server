#ifndef PERSISTENCE_PERSISTENCE_MANAGER_H
#define PERSISTENCE_PERSISTENCE_MANAGER_H

#include <functional>
#include <string>
#include <vector>

// A single key/value record exchanged with the persistence layer.
//
// Persistence deals in flat records, not in Database internals (Entry, TTLs,
// hash buckets), so the storage format stays decoupled from the in-memory
// representation.
struct Record
{
    std::string key;
    std::string value;
};

// A single durable mutation, as seen by the persistence layer. This is what an
// append-only strategy writes to its log; a snapshot strategy ignores it.
struct WriteOp
{
    enum class Type
    {
        Set,    // set `key` to `value`
        Delete, // remove `key`
        Clear,  // remove every key (FLUSHDB / FLUSHALL)
    };

    Type type;
    std::string key;
    std::string value;
};

// Lazily yields the complete current dataset. Strategies that need the full
// state (snapshot, AOF compaction) call it; append-only writes do not, so the
// snapshot is never built on the hot path when it is not required.
using DatasetProvider = std::function<std::vector<Record>()>;

// Strategy interface for making the keyspace durable.
//
// StorageEngine depends only on this abstraction, never on a concrete file
// format. Two implementations exist: SnapshotPersistence (rewrites the whole
// dataset) and AofPersistence (appends each mutation to a log and replays it).
// Adding another strategy is a localized change behind this interface.
class PersistenceManager
{
public:
    virtual ~PersistenceManager() = default;

    // Startup: reconstruct and return everything persisted by a previous run.
    // Returns an empty vector when nothing has been stored yet (first run).
    virtual std::vector<Record> load() = 0;

    // Called after each committed write. `ops` describes the incremental change;
    // `dataset` lazily provides the full current state for strategies that need
    // it (snapshot writes it; AOF appends the ops and ignores it).
    virtual void record(const std::vector<WriteOp> &ops, const DatasetProvider &dataset) = 0;

    // Force a full durable checkpoint: a snapshot rewrite, or AOF compaction.
    virtual void checkpoint(const DatasetProvider &dataset) = 0;
};

#endif  // PERSISTENCE_PERSISTENCE_MANAGER_H
