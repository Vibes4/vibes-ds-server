#include "redis/commands/get_command.h"

CommandResult GetCommand::execute(KVStore &store, const std::string &key,
                                  const std::string & /*value*/)
{
    if (key.empty())
    {
        return {false, "GET requires a key"};
    }
    return {true, store.get(key)};
}
