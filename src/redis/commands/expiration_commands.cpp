#include "redis/commands/expiration_commands.h"

#include "redis/command_util.h"

CommandResult ExpireCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.size() != 2)
    {
        return wrong_args("expire");
    }
    const auto seconds = parse_ll(args[1]);
    if (!seconds)
    {
        return {false, "value is not an integer or out of range"};
    }
    const bool ok = store.with_lock(
        [&](Database &db) { return db.set_expiry_ms(args[0], *seconds * 1000); });
    return {true, ok ? "1" : "0"};
}

CommandResult PExpireCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.size() != 2)
    {
        return wrong_args("pexpire");
    }
    const auto millis = parse_ll(args[1]);
    if (!millis)
    {
        return {false, "value is not an integer or out of range"};
    }
    const bool ok = store.with_lock(
        [&](Database &db) { return db.set_expiry_ms(args[0], *millis); });
    return {true, ok ? "1" : "0"};
}

CommandResult TtlCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.size() != 1)
    {
        return wrong_args("ttl");
    }
    const long long ms = store.with_lock([&](Database &db) { return db.pttl_ms(args[0]); });
    if (ms < 0)
    {
        return {true, std::to_string(ms)};  // -1 or -2 pass through unchanged
    }
    return {true, std::to_string((ms + 500) / 1000)};  // round to nearest second
}

CommandResult PTtlCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.size() != 1)
    {
        return wrong_args("pttl");
    }
    const long long ms = store.with_lock([&](Database &db) { return db.pttl_ms(args[0]); });
    return {true, std::to_string(ms)};
}

CommandResult PersistCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.size() != 1)
    {
        return wrong_args("persist");
    }
    const bool ok = store.with_lock([&](Database &db) { return db.clear_expiry(args[0]); });
    return {true, ok ? "1" : "0"};
}
