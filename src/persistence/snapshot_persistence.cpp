#include "persistence/snapshot_persistence.h"

#include "observability/logger.h"

#include <fstream>
#include <utility>

SnapshotPersistence::SnapshotPersistence(std::string file_path)
    : file_path_(std::move(file_path))
{
}

std::vector<Record> SnapshotPersistence::load()
{
    std::vector<Record> records;

    std::ifstream file(file_path_);
    if (!file.is_open())
    {
        return records;  // first run: nothing has been persisted yet
    }

    std::string key, value;
    while (file >> key >> value)
    {
        records.push_back({key, value});
    }
    return records;
}

void SnapshotPersistence::save(const std::vector<Record> &records)
{
    std::ofstream file(file_path_, std::ios::trunc);
    if (!file.is_open())
    {
        Logger::error("persistence", "could not open " + file_path_ + " for writing");
        return;
    }

    for (const auto &record : records)
    {
        file << record.key << " " << record.value << "\n";
    }
}
