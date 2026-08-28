#ifndef REDIS_COMMANDS_NUMERIC_COMMANDS_H
#define REDIS_COMMANDS_NUMERIC_COMMANDS_H

#include "redis/command.h"

// Numeric commands treat the stored string as a base-10 integer. A missing key
// is treated as 0. They reply with the new value, or an error if the current
// value is not an integer.

// INCR key -> value + 1.
class IncrCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// INCRBY key amount -> value + amount.
class IncrByCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// DECR key -> value - 1.
class DecrCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// DECRBY key amount -> value - amount.
class DecrByCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

#endif  // REDIS_COMMANDS_NUMERIC_COMMANDS_H
