#include "redis/commands/del_command.h"

CommandResult DelCommand::execute(KVStore &store, const std::string &key,
                                  const std::string & /*value*/)
{
    if (key.empty())
    {
        return {false, "DEL requires a key"};
    }
    return {true, store.del(key) ? "Deleted" : "(nil)"};
}
