#ifndef REDIS_COMMANDS_STRING_COMMANDS_H
#define REDIS_COMMANDS_STRING_COMMANDS_H

#include "redis/command.h"

// String commands operate on plain string values. Each class handles one
// command; the reply conventions are noted next to each.

// SET key value -> "OK". Overwrites any existing value and clears its TTL.
class SetCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// GET key -> the value, or "(nil)" if the key is absent.
class GetCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// DEL key [key ...] -> the number of keys that were removed.
class DelCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// EXISTS key [key ...] -> how many of the given keys exist.
class ExistsCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// MSET key value [key value ...] -> "OK". Sets several pairs at once.
class MSetCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// MGET key [key ...] -> one value (or "(nil)") per line.
class MGetCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// APPEND key value -> the new string length after appending.
class AppendCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// STRLEN key -> the length of the string (0 if the key is absent).
class StrlenCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// GETSET key value -> the previous value (or "(nil)"), and stores the new one.
class GetSetCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// SETNX key value -> 1 if stored, 0 if the key already existed.
class SetNxCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// SETEX key seconds value -> "OK". Stores the value with a TTL.
class SetExCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// GETDEL key -> the value (or "(nil)"), and removes the key.
class GetDelCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

#endif  // REDIS_COMMANDS_STRING_COMMANDS_H
