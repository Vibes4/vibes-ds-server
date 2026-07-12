#ifndef REDIS_COMMANDS_KEY_COMMANDS_H
#define REDIS_COMMANDS_KEY_COMMANDS_H

#include "redis/command.h"

// Key-space commands inspect or manage keys regardless of their value.

// KEYS pattern -> matching keys, one per line ("(empty)" if none).
// Supports the glob wildcards '*' and '?'.
class KeysCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// SCAN cursor -> "cursor" line followed by keys. This is a simplified,
// non-incremental scan: it always returns every key with a next cursor of 0.
class ScanCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// TYPE key -> "string" (the only supported type) or "none" if absent.
class TypeCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// RENAME key newkey -> "OK", or an error if the source key is missing.
class RenameCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// RANDOMKEY -> a key from the store, or "(nil)" if it is empty.
class RandomKeyCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

#endif  // REDIS_COMMANDS_KEY_COMMANDS_H
