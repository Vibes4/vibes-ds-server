#include "redis/commands/string_commands.h"

#include "redis/command_util.h"

#include <optional>
#include <utility>

CommandResult SetCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.size() != 2)
    {
        return wrong_args("set");
    }
    store.with_lock([&](Database &db) { db.set_value(args[0], args[1]); });
    return {true, "OK"};
}

CommandResult GetCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.size() != 1)
    {
        return wrong_args("get");
    }
    const auto value = store.with_lock([&](Database &db) { return db.get_value(args[0]); });
    return {true, value.value_or("(nil)")};
}

CommandResult DelCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.empty())
    {
        return wrong_args("del");
    }
    const size_t removed = store.with_lock([&](Database &db) {
        size_t count = 0;
        for (const auto &key : args)
        {
            if (db.erase(key))
            {
                ++count;
            }
        }
        return count;
    });
    return {true, std::to_string(removed)};
}

CommandResult ExistsCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.empty())
    {
        return wrong_args("exists");
    }
    const size_t found = store.with_lock([&](Database &db) {
        size_t count = 0;
        for (const auto &key : args)
        {
            if (db.contains(key))
            {
                ++count;
            }
        }
        return count;
    });
    return {true, std::to_string(found)};
}

CommandResult MSetCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.empty() || args.size() % 2 != 0)
    {
        return wrong_args("mset");
    }
    store.with_lock([&](Database &db) {
        for (size_t i = 0; i < args.size(); i += 2)
        {
            db.set_value(args[i], args[i + 1]);
        }
    });
    return {true, "OK"};
}

CommandResult MGetCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.empty())
    {
        return wrong_args("mget");
    }
    const auto values = store.with_lock([&](Database &db) {
        std::vector<std::optional<std::string>> result;
        result.reserve(args.size());
        for (const auto &key : args)
        {
            result.push_back(db.get_value(key));
        }
        return result;
    });

    std::string reply;
    for (size_t i = 0; i < values.size(); ++i)
    {
        reply += values[i].value_or("(nil)");
        if (i + 1 < values.size())
        {
            reply += "\n";
        }
    }
    return {true, reply};
}

CommandResult AppendCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.size() != 2)
    {
        return wrong_args("append");
    }
    const size_t length = store.with_lock([&](Database &db) {
        std::string value = db.get_value(args[0]).value_or("");
        value += args[1];
        const size_t new_length = value.size();
        db.set_value_keep_ttl(args[0], std::move(value));
        return new_length;
    });
    return {true, std::to_string(length)};
}

CommandResult StrlenCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.size() != 1)
    {
        return wrong_args("strlen");
    }
    const size_t length = store.with_lock([&](Database &db) {
        const auto value = db.get_value(args[0]);
        return value ? value->size() : size_t{0};
    });
    return {true, std::to_string(length)};
}

CommandResult GetSetCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.size() != 2)
    {
        return wrong_args("getset");
    }
    const auto old = store.with_lock([&](Database &db) {
        auto previous = db.get_value(args[0]);
        db.set_value(args[0], args[1]);
        return previous;
    });
    return {true, old.value_or("(nil)")};
}

CommandResult SetNxCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.size() != 2)
    {
        return wrong_args("setnx");
    }
    const bool stored = store.with_lock([&](Database &db) {
        if (db.contains(args[0]))
        {
            return false;
        }
        db.set_value(args[0], args[1]);
        return true;
    });
    return {true, stored ? "1" : "0"};
}

CommandResult SetExCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.size() != 3)
    {
        return wrong_args("setex");
    }
    const auto seconds = parse_ll(args[1]);
    if (!seconds || *seconds <= 0)
    {
        return {false, "invalid expire time in 'setex'"};
    }
    store.with_lock([&](Database &db) {
        db.set_value(args[0], args[2]);
        db.set_expiry_ms(args[0], *seconds * 1000);
    });
    return {true, "OK"};
}

CommandResult GetDelCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.size() != 1)
    {
        return wrong_args("getdel");
    }
    const auto value = store.with_lock([&](Database &db) {
        auto current = db.get_value(args[0]);
        if (current)
        {
            db.erase(args[0]);
        }
        return current;
    });
    return {true, value.value_or("(nil)")};
}
