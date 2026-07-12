#ifndef PERSISTENCE_SNAPSHOT_PERSISTENCE_H
#define PERSISTENCE_SNAPSHOT_PERSISTENCE_H

#include "persistence/persistence_manager.h"
#include <string>

// Persists the keyspace by rewriting a full snapshot to a text file: one
// "key value" pair per line, the whole file replaced on every save.
//
// The file always reflects a complete, consistent view of the data, which makes
// recovery trivial. The trade-off is O(N) work per save; an append-only
// strategy would trade that for a log that must be replayed and compacted.
class SnapshotPersistence : public PersistenceManager
{
public:
    explicit SnapshotPersistence(std::string file_path);

    std::vector<Record> load() override;
    void save(const std::vector<Record> &records) override;

private:
    std::string file_path_;
};

#endif  // PERSISTENCE_SNAPSHOT_PERSISTENCE_H
