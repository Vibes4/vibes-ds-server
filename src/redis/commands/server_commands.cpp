#include "redis/commands/server_commands.h"

#include "redis/command_util.h"

#include <sstream>

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
    const InfoSnapshot s = store.info_snapshot();

    std::ostringstream out;
    out << "# Server\n"
        << "server_name:vibes-ds-server\n"
        << "version:0.7\n"
        << "uptime_seconds:" << s.uptime_seconds << "\n"
        << "start_time:" << s.start_epoch << "\n"
        << "\n# Memory\n"
        << "dataset_bytes:" << s.dataset_bytes << "\n"
        << "key_count:" << s.key_count << "\n"
        << "\n# Stats\n"
        << "total_commands:" << s.total_commands << "\n"
        << "total_reads:" << s.total_reads << "\n"
        << "total_writes:" << s.total_writes << "\n"
        << "cache_hits:" << s.cache_hits << "\n"
        << "cache_misses:" << s.cache_misses << "\n"
        << "expired_keys_lazy:" << s.expired_lazy << "\n"
        << "expired_keys_sweeper:" << s.expired_sweeper << "\n"
        << "avg_command_latency_us:" << s.avg_command_latency_us << "\n"
        << "\n# Persistence\n"
        << "persistence_ops:" << s.persistence_ops << "\n"
        << "last_persistence_time:" << s.last_persistence_epoch << "\n"
        << "\n# Keyspace\n"
        << "db0:keys=" << s.key_count << "\n";
    return {true, out.str()};
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
