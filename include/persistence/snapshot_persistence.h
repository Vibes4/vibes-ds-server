#ifndef PERSISTENCE_SNAPSHOT_PERSISTENCE_H
#define PERSISTENCE_SNAPSHOT_PERSISTENCE_H

#include "persistence/persistence_manager.h"
#include <string>

// Persists the keyspace by rewriting a full snapshot to a text file: one
// "key value" pair per line, the whole file replaced on every write.
//
// The file always reflects a complete, consistent view of the data, which makes
// recovery trivial. The trade-off is O(N) work per write. It ignores the
// per-mutation `ops` and simply writes the current dataset.
class SnapshotPersistence : public PersistenceManager
{
public:
    explicit SnapshotPersistence(std::string file_path);

    std::vector<Record> load() override;
    void record(const std::vector<WriteOp> &ops, const DatasetProvider &dataset) override;
    void checkpoint(const DatasetProvider &dataset) override;

private:
    void write(const std::vector<Record> &records);

    std::string file_path_;
};

#endif  // PERSISTENCE_SNAPSHOT_PERSISTENCE_H
