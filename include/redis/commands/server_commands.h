#ifndef REDIS_COMMANDS_SERVER_COMMANDS_H
#define REDIS_COMMANDS_SERVER_COMMANDS_H

#include "redis/command.h"

// Server/administrative commands.

// PING [message] -> "PONG", or the message echoed back if one is given.
class PingCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// ECHO message -> the message, unchanged.
class EchoCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// INFO -> a small block of server and keyspace statistics.
class InfoCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// DBSIZE -> the number of keys currently stored.
class DbSizeCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// FLUSHDB -> "OK". Removes every key.
class FlushDbCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// FLUSHALL -> "OK". Removes every key (this server has a single database).
class FlushAllCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

#endif  // REDIS_COMMANDS_SERVER_COMMANDS_H
