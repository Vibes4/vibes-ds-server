#ifndef REDIS_REDIS_SERVICE_H
#define REDIS_REDIS_SERVICE_H

#include "redis/command.h"
#include "store/kv_store.h"
#include <map>
#include <memory>
#include <string>

// Dispatches Redis-style commands to their controllers.
//
// It owns the key-value store and a registry mapping command names to the
// controller that handles them. Adding a new command means writing one
// controller file and registering it in the constructor; nothing else changes.
class RedisService
{
public:
    RedisService();

    // Looks up the controller for `command` (case-insensitive) and runs it.
    // Returns an error result if no controller is registered for the command.
    CommandResult execute(const std::string &command,
                          const std::string &key,
                          const std::string &value);

    // Direct store access for endpoints that need the full data set.
    KVStore &store() { return store_; }

private:
    void register_command(const std::string &name, std::unique_ptr<Command> command);

    KVStore store_;
    std::map<std::string, std::unique_ptr<Command>> commands_;
};

#endif  // REDIS_REDIS_SERVICE_H
