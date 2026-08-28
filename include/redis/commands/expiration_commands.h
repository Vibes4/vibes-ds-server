#ifndef REDIS_COMMANDS_EXPIRATION_COMMANDS_H
#define REDIS_COMMANDS_EXPIRATION_COMMANDS_H

#include "redis/command.h"

// Expiration commands manage per-key time-to-live (TTL).

// EXPIRE key seconds -> 1 if the TTL was set, 0 if the key is missing.
class ExpireCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// PEXPIRE key milliseconds -> 1 if the TTL was set, 0 if the key is missing.
class PExpireCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// TTL key -> remaining seconds, -1 if no expiry, -2 if the key is missing.
class TtlCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// PTTL key -> remaining milliseconds, -1 if no expiry, -2 if the key is missing.
class PTtlCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// PERSIST key -> 1 if an expiry was removed, 0 otherwise.
class PersistCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

#endif  // REDIS_COMMANDS_EXPIRATION_COMMANDS_H
