#ifndef REDIS_COMMANDS_EXISTS_COMMAND_H
#define REDIS_COMMANDS_EXISTS_COMMAND_H

#include "redis/command.h"

// EXISTS <key> -> replies "1" if the key exists, otherwise "0".
class ExistsCommand : public Command
{
public:
    CommandResult execute(KVStore &store, const std::string &key,
                          const std::string &value) override;
};

#endif  // REDIS_COMMANDS_EXISTS_COMMAND_H
