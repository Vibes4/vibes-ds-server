#ifndef REDIS_COMMANDS_DEL_COMMAND_H
#define REDIS_COMMANDS_DEL_COMMAND_H

#include "redis/command.h"

// DEL <key> -> removes the key, replies "Deleted" or "(nil)" if absent.
class DelCommand : public Command
{
public:
    CommandResult execute(KVStore &store, const std::string &key,
                          const std::string &value) override;
};

#endif  // REDIS_COMMANDS_DEL_COMMAND_H
