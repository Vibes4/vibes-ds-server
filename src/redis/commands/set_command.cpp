#include "redis/commands/set_command.h"

CommandResult SetCommand::execute(KVStore &store, const std::string &key,
                                  const std::string &value)
{
    if (key.empty() || value.empty())
    {
        return {false, "SET requires a key and a value"};
    }
    store.set(key, value);
    return {true, "OK"};
}
