#include "redis/command_executor.h"

#include "observability/logger.h"
#include "redis/commands/expiration_commands.h"
#include "redis/commands/key_commands.h"
#include "redis/commands/numeric_commands.h"
#include "redis/commands/persistence_commands.h"
#include "redis/commands/server_commands.h"
#include "redis/commands/string_commands.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <sstream>

namespace
{

std::string to_upper(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return text;
}

// Splits a command line into whitespace-separated tokens.
std::vector<std::string> tokenize(const std::string &line)
{
    std::istringstream stream(line);
    std::vector<std::string> tokens;
    std::string token;
    while (stream >> token)
    {
        tokens.push_back(token);
    }
    return tokens;
}

} // namespace

CommandExecutor::CommandExecutor()
{
    // Register one controller per command, grouped by category. This is the
    // single source of truth for which commands the server understands.

    // Strings
    register_command("SET", std::make_unique<SetCommand>());
    register_command("GET", std::make_unique<GetCommand>());
    register_command("DEL", std::make_unique<DelCommand>());
    register_command("EXISTS", std::make_unique<ExistsCommand>());
    register_command("MSET", std::make_unique<MSetCommand>());
    register_command("MGET", std::make_unique<MGetCommand>());
    register_command("APPEND", std::make_unique<AppendCommand>());
    register_command("STRLEN", std::make_unique<StrlenCommand>());
    register_command("GETSET", std::make_unique<GetSetCommand>());
    register_command("SETNX", std::make_unique<SetNxCommand>());
    register_command("SETEX", std::make_unique<SetExCommand>());
    register_command("GETDEL", std::make_unique<GetDelCommand>());

    // Numeric
    register_command("INCR", std::make_unique<IncrCommand>());
    register_command("INCRBY", std::make_unique<IncrByCommand>());
    register_command("DECR", std::make_unique<DecrCommand>());
    register_command("DECRBY", std::make_unique<DecrByCommand>());

    // Expiration
    register_command("EXPIRE", std::make_unique<ExpireCommand>());
    register_command("PEXPIRE", std::make_unique<PExpireCommand>());
    register_command("TTL", std::make_unique<TtlCommand>());
    register_command("PTTL", std::make_unique<PTtlCommand>());
    register_command("PERSIST", std::make_unique<PersistCommand>());

    // Keys
    register_command("KEYS", std::make_unique<KeysCommand>());
    register_command("SCAN", std::make_unique<ScanCommand>());
    register_command("TYPE", std::make_unique<TypeCommand>());
    register_command("RENAME", std::make_unique<RenameCommand>());
    register_command("RANDOMKEY", std::make_unique<RandomKeyCommand>());

    // Server
    register_command("PING", std::make_unique<PingCommand>());
    register_command("ECHO", std::make_unique<EchoCommand>());
    register_command("INFO", std::make_unique<InfoCommand>());
    register_command("DBSIZE", std::make_unique<DbSizeCommand>());
    register_command("FLUSHDB", std::make_unique<FlushDbCommand>());
    register_command("FLUSHALL", std::make_unique<FlushAllCommand>());

    // Persistence
    register_command("SAVE", std::make_unique<SaveCommand>());
    register_command("BGSAVE", std::make_unique<BgSaveCommand>());
    register_command("LASTSAVE", std::make_unique<LastSaveCommand>());
}

void CommandExecutor::register_command(const std::string &name,
                                    std::unique_ptr<Command> command)
{
    commands_[name] = std::move(command);
}

CommandResult CommandExecutor::execute(const std::string &command_line)
{
    const auto started = std::chrono::steady_clock::now();
    const std::vector<std::string> tokens = tokenize(command_line);

    // Tokenise -> look up -> execute. The result is computed first so we can
    // time and log every path (empty, unknown, ok, error) uniformly below.
    const CommandResult result = [&]() -> CommandResult {
        if (tokens.empty())
        {
            return {false, "empty command"};
        }
        const auto it = commands_.find(to_upper(tokens.front()));
        if (it == commands_.end())
        {
            return {false, "unknown command '" + tokens.front() + "'"};
        }
        const std::vector<std::string> args(tokens.begin() + 1, tokens.end());
        return it->second->execute(store_, args);
    }();

    const auto latency = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - started);
    store_.record_command(latency);

    const std::string name = tokens.empty() ? "(empty)" : tokens.front();
    if (result.ok)
    {
        Logger::debug("command", name + " (" + std::to_string(latency.count()) + "us)");
    }
    else
    {
        Logger::warn("command", name + " failed: " + result.message);
    }
    return result;
}
