#ifndef PERSISTENCE_PERSISTENCE_MANAGER_H
#define PERSISTENCE_PERSISTENCE_MANAGER_H

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

// Strategy interface for making the keyspace durable.
//
// The database engine (StorageEngine) depends only on this abstraction, never on a
// concrete file format. Today the sole implementation is SnapshotPersistence,
// which writes a full snapshot of the dataset. A future append-only strategy
// (AOF) can be added by implementing this interface and injecting it into
// StorageEngine -- no engine, command, or transport code has to change.
//
// Extension note: an append-only strategy will also want per-mutation hooks
// (e.g. append(op)). Those can be added to this interface later; Snapshot
// persistence would ignore them and keep rewriting on save(). The point of this
// seam is that adding a strategy is a localized change behind this interface.
class PersistenceManager
{
public:
    virtual ~PersistenceManager() = default;

    // Reconstructs and returns everything persisted by a previous run.
    // Returns an empty vector when nothing has been stored yet (first run).
    virtual std::vector<Record> load() = 0;

    // Durably stores the complete current dataset.
    virtual void save(const std::vector<Record> &records) = 0;
};

#endif  // PERSISTENCE_PERSISTENCE_MANAGER_H
