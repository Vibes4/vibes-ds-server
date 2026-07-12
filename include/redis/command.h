#ifndef REDIS_COMMAND_H
#define REDIS_COMMAND_H

#include "store/kv_store.h"
#include <string>

// The outcome of executing a Redis-style command.
struct CommandResult
{
    bool ok;              // false for a client error (bad/unknown command)
    std::string message;  // reply text, e.g. "OK", a value, or an error
};

// Interface implemented by every command controller (SET, GET, DEL, ...).
//
// Each command lives in its own controller file under redis/commands/ and
// operates on the shared key-value store. Controllers are stateless: the store
// is passed in per call, so a single instance can serve every request.
class Command
{
public:
    virtual ~Command() = default;

    virtual CommandResult execute(KVStore &store,
                                  const std::string &key,
                                  const std::string &value) = 0;
};

#endif  // REDIS_COMMAND_H
