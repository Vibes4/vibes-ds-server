#include "redis/commands/exists_command.h"

CommandResult ExistsCommand::execute(KVStore &store, const std::string &key,
                                     const std::string & /*value*/)
{
    if (key.empty())
    {
        return {false, "EXISTS requires a key"};
    }
    return {true, store.exists(key) ? "1" : "0"};
}
