// Enable POSIX declarations (fileno/fsync) under -std=c++17. Must precede any
// system header, so it is the very first thing in the translation unit.
#if !defined(_WIN32) && !defined(_WIN64)
#define _POSIX_C_SOURCE 200809L
#endif

#include "persistence/aof_persistence.h"

#include "observability/logger.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

#if defined(_WIN32) || defined(_WIN64)
#include <io.h> // _commit, _fileno
#else
#include <unistd.h> // fsync
#endif

AofPersistence::AofPersistence(std::string file_path, Sync sync,
                               std::size_t rewrite_threshold)
    : file_path_(std::move(file_path)), sync_(sync), rewrite_threshold_(rewrite_threshold)
{
    open_for_append();
}

AofPersistence::~AofPersistence()
{
    if (log_ != nullptr)
    {
        std::fclose(log_);
    }
}

void AofPersistence::open_for_append()
{
    log_ = std::fopen(file_path_.c_str(), "ab");
    if (log_ == nullptr)
    {
        Logger::error("persistence", "could not open AOF file " + file_path_);
    }
}

std::vector<Record> AofPersistence::load()
{
    std::ifstream file(file_path_);
    if (!file.is_open())
    {
        return {};  // first run: nothing has been persisted yet
    }

    // Replay the log into a map so repeated writes to the same key collapse.
    std::unordered_map<std::string, std::string> state;
    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
        {
            continue;
        }
        std::istringstream stream(line);
        std::string op;
        stream >> op;
        if (op == "SET")
        {
            std::string key, value;
            stream >> key >> value;
            state[key] = value;
        }
        else if (op == "DEL")
        {
            std::string key;
            stream >> key;
            state.erase(key);
        }
        else if (op == "CLR")
        {
            state.clear();
        }
    }

    std::vector<Record> records;
    records.reserve(state.size());
    for (const auto &[key, value] : state)
    {
        records.push_back({key, value});
    }
    return records;
}

void AofPersistence::record(const std::vector<WriteOp> &ops, const DatasetProvider &dataset)
{
    if (log_ == nullptr)
    {
        return;
    }
    for (const auto &op : ops)
    {
        append_line(op);
    }
    sync_to_disk();

    ops_since_rewrite_ += ops.size();
    if (rewrite_threshold_ > 0 && ops_since_rewrite_ >= rewrite_threshold_)
    {
        rewrite(dataset());  // automatic compaction, like BGREWRITEAOF
    }
}

void AofPersistence::checkpoint(const DatasetProvider &dataset)
{
    rewrite(dataset());
}

void AofPersistence::append_line(const WriteOp &op)
{
    std::string line;
    switch (op.type)
    {
    case WriteOp::Type::Set:
        line = "SET " + op.key + " " + op.value + "\n";
        break;
    case WriteOp::Type::Delete:
        line = "DEL " + op.key + "\n";
        break;
    case WriteOp::Type::Clear:
        line = "CLR\n";
        break;
    }
    std::fwrite(line.data(), 1, line.size(), log_);
}

void AofPersistence::sync_to_disk()
{
    if (log_ == nullptr)
    {
        return;
    }
    std::fflush(log_);
    if (sync_ != Sync::Always)
    {
        return;
    }
#if defined(_WIN32) || defined(_WIN64)
    _commit(_fileno(log_));
#else
    ::fsync(::fileno(log_));
#endif
}

void AofPersistence::rewrite(const std::vector<Record> &records)
{
    // Write a compacted log to a temp file, then atomically replace the log.
    const std::string temp_path = file_path_ + ".tmp";
    {
        std::ofstream out(temp_path, std::ios::trunc);
        if (!out.is_open())
        {
            Logger::error("persistence", "could not open " + temp_path + " for AOF rewrite");
            return;
        }
        for (const auto &record : records)
        {
            out << "SET " << record.key << " " << record.value << "\n";
        }
    }

    if (log_ != nullptr)
    {
        std::fclose(log_);
        log_ = nullptr;
    }
    if (std::rename(temp_path.c_str(), file_path_.c_str()) != 0)
    {
        Logger::error("persistence", "AOF rewrite could not replace " + file_path_);
    }
    open_for_append();
    ops_since_rewrite_ = 0;
    Logger::info("persistence", "AOF compacted to " + std::to_string(records.size()) + " key(s)");
}
