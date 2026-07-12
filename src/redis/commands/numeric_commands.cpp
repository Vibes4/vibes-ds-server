#include "redis/commands/numeric_commands.h"

#include "redis/command_util.h"

namespace
{

// The shared INCR/DECR body: atomically read the value as an integer, add
// `delta`, store it back (keeping any TTL), and format the reply.
CommandResult apply_delta(StorageEngine &store, const std::string &key, long long delta)
{
    return store.with_lock([&](Database &db) -> CommandResult {
        long long current = 0;
        if (const auto value = db.get_value(key))
        {
            const auto parsed = parse_ll(*value);
            if (!parsed)
            {
                return {false, "value is not an integer or out of range"};
            }
            current = *parsed;
        }
        current += delta;
        db.set_value_keep_ttl(key, std::to_string(current));
        return {true, std::to_string(current)};
    });
}

} // namespace

CommandResult IncrCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.size() != 1)
    {
        return wrong_args("incr");
    }
    return apply_delta(store, args[0], 1);
}

CommandResult IncrByCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.size() != 2)
    {
        return wrong_args("incrby");
    }
    const auto amount = parse_ll(args[1]);
    if (!amount)
    {
        return {false, "value is not an integer or out of range"};
    }
    return apply_delta(store, args[0], *amount);
}

CommandResult DecrCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.size() != 1)
    {
        return wrong_args("decr");
    }
    return apply_delta(store, args[0], -1);
}

CommandResult DecrByCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    if (args.size() != 2)
    {
        return wrong_args("decrby");
    }
    const auto amount = parse_ll(args[1]);
    if (!amount)
    {
        return {false, "value is not an integer or out of range"};
    }
    return apply_delta(store, args[0], -*amount);
}
