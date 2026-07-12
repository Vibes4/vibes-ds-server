#include "redis/commands/key_commands.h"

#include "redis/command_util.h"

namespace
{

// Joins keys one per line, or a placeholder when there are none.
std::string join_lines(const std::vector<std::string> &keys)
{
    if (keys.empty())
    {
        return "(empty)";
    }
    std::string reply;
    for (size_t i = 0; i < keys.size(); ++i)
    {
        reply += keys[i];
        if (i + 1 < keys.size())
        {
            reply += "\n";
        }
    }
    return reply;
}

} // namespace

CommandResult KeysCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.size() != 1)
    {
        return wrong_args("keys");
    }
    const auto matches = store.with_lock([&](Database &db) { return db.keys(args[0]); });
    return {true, join_lines(matches)};
}

CommandResult ScanCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.size() != 1)
    {
        return wrong_args("scan");
    }
    // A real SCAN is incremental; this simplified version returns everything at
    // once and reports the next cursor as 0 (meaning "iteration complete").
    const auto keys = store.with_lock([](Database &db) { return db.keys("*"); });
    std::string reply = "0";
    for (const auto &key : keys)
    {
        reply += "\n" + key;
    }
    return {true, reply};
}

CommandResult TypeCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.size() != 1)
    {
        return wrong_args("type");
    }
    const bool exists = store.with_lock([&](Database &db) { return db.contains(args[0]); });
    return {true, exists ? "string" : "none"};
}

CommandResult RenameCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.size() != 2)
    {
        return wrong_args("rename");
    }
    const bool ok = store.with_lock([&](Database &db) { return db.rename(args[0], args[1]); });
    if (!ok)
    {
        return {false, "no such key"};
    }
    return {true, "OK"};
}

CommandResult RandomKeyCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (!args.empty())
    {
        return wrong_args("randomkey");
    }
    const auto key = store.with_lock([](Database &db) { return db.random_key(); });
    return {true, key.value_or("(nil)")};
}
