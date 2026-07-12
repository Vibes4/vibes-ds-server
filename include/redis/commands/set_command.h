#ifndef REDIS_COMMANDS_SET_COMMAND_H
#define REDIS_COMMANDS_SET_COMMAND_H

#include "redis/command.h"

// SET <key> <value> -> stores the value, replies "OK".
class SetCommand : public Command
{
public:
    CommandResult execute(KVStore &store, const std::string &key,
                          const std::string &value) override;
};

#endif  // REDIS_COMMANDS_SET_COMMAND_H
