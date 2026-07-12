#include "redis/redis_service.h"

#include "redis/commands/del_command.h"
#include "redis/commands/exists_command.h"
#include "redis/commands/get_command.h"
#include "redis/commands/set_command.h"

#include <algorithm>
#include <cctype>

namespace
{

    std::string to_upper(std::string text)
    {
        std::transform(text.begin(), text.end(), text.begin(),
                       [](unsigned char c)
                       { return std::toupper(c); });
        return text;
    }

} // namespace

RedisService::RedisService()
{
    // Register one controller per command. New commands are added here.
    register_command("SET", std::make_unique<SetCommand>());
    register_command("GET", std::make_unique<GetCommand>());
    register_command("DEL", std::make_unique<DelCommand>());
    register_command("EXISTS", std::make_unique<ExistsCommand>());
}

void RedisService::register_command(const std::string &name,
                                    std::unique_ptr<Command> command)
{
    commands_[name] = std::move(command);
}

CommandResult RedisService::execute(
    const std::string &command,
    const std::string &key,
    const std::string &value)
{
    const auto it = commands_.find(to_upper(command));
    if (it == commands_.end())
    {
        return {false, "Invalid Command"};
    }
    return it->second->execute(store_, key, value);
}
