#ifndef REDIS_COMMANDS_GET_COMMAND_H
#define REDIS_COMMANDS_GET_COMMAND_H

#include "redis/command.h"

// GET <key> -> replies with the stored value, or "(nil)" if absent.
class GetCommand : public Command
{
public:
    CommandResult execute(KVStore &store, const std::string &key,
                          const std::string &value) override;
};

#endif  // REDIS_COMMANDS_GET_COMMAND_H
