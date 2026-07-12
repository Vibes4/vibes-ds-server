#include "redis/commands/server_commands.h"

#include "redis/command_util.h"

CommandResult PingCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    (void)store;
    if (args.empty())
    {
        return {true, "PONG"};
    }
    if (args.size() == 1)
    {
        return {true, args[0]};
    }
    return wrong_args("ping");
}

CommandResult EchoCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    (void)store;
    if (args.size() != 1)
    {
        return wrong_args("echo");
    }
    return {true, args[0]};
}

CommandResult InfoCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    (void)args;
    const size_t keys = store.with_lock([](Database &db) { return db.size(); });
    const std::string info =
        "# Server\n"
        "server_name:vibes-ds-server\n"
        "version:0.6\n"
        "# Keyspace\n"
        "db0:keys=" + std::to_string(keys) + "\n";
    return {true, info};
}

CommandResult DbSizeCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    (void)args;
    const size_t keys = store.with_lock([](Database &db) { return db.size(); });
    return {true, std::to_string(keys)};
}

CommandResult FlushDbCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    (void)args;
    store.with_lock([](Database &db) { db.clear(); });
    return {true, "OK"};
}

CommandResult FlushAllCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    (void)args;
    store.with_lock([](Database &db) { db.clear(); });
    return {true, "OK"};
}
