#ifndef PERSISTENCE_AOF_PERSISTENCE_H
#define PERSISTENCE_AOF_PERSISTENCE_H

#include "persistence/persistence_manager.h"
#include <cstddef>
#include <cstdio>
#include <string>

// Append-only-file persistence.
//
// Every mutation is appended to a log as a single line (SET/DEL/CLR); on
// startup the log is replayed to rebuild the dataset. This makes writes cheap
// and durable (no full rewrite per change), at the cost of a log that grows
// until it is compacted. Compaction (`checkpoint`, or automatically once the
// log passes a threshold) rewrites the log as one SET per live key.
//
// Durability is controlled by the sync policy: `Always` fsyncs after every
// write batch; `Never` leaves flushing to the OS (faster, weaker guarantee).
//
// Note: TTLs are not persisted (expiry is in-memory only), so a key that
// expired but was never overwritten or deleted reappears (without its TTL) after
// a restart, until the next compaction drops it.
class AofPersistence : public PersistenceManager
{
public:
    enum class Sync
    {
        Always, // fsync after every write batch (durable)
        Never,  // let the OS flush (faster, weaker durability)
    };

    explicit AofPersistence(std::string file_path, Sync sync = Sync::Always,
                            std::size_t rewrite_threshold = 1000);
    ~AofPersistence() override;

    AofPersistence(const AofPersistence &) = delete;
    AofPersistence &operator=(const AofPersistence &) = delete;

    std::vector<Record> load() override;
    void record(const std::vector<WriteOp> &ops, const DatasetProvider &dataset) override;
    void checkpoint(const DatasetProvider &dataset) override;

private:
    void open_for_append();
    void append_line(const WriteOp &op);
    void sync_to_disk();
    void rewrite(const std::vector<Record> &records);

    std::string file_path_;
    Sync sync_;
    std::size_t rewrite_threshold_;
    std::size_t ops_since_rewrite_ = 0;
    std::FILE *log_ = nullptr;
};

#endif  // PERSISTENCE_AOF_PERSISTENCE_H
