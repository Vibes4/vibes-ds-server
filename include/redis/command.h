#ifndef REDIS_COMMAND_H
#define REDIS_COMMAND_H

#include "store/storage_engine.h"
#include <string>
#include <vector>

// The outcome of executing a Redis-style command.
struct CommandResult
{
    bool ok;              // false for a client error (bad args / unknown command)
    std::string message;  // reply text: "OK", a value, an integer, or an error
};

// Interface implemented by every command controller (SET, GET, INCR, ...).
//
// Controllers are grouped by category (string, numeric, expiration, ...) but
// each command is its own class. They are stateless: the store and the parsed
// argument list are passed in per call, so one instance serves every request.
//
// `args` holds the arguments *after* the command name, e.g. for "SET a 1" it is
// {"a", "1"}.
class Command
{
public:
    virtual ~Command() = default;

    virtual CommandResult execute(StorageEngine &store,
                                  const std::vector<std::string> &args) = 0;
};

#endif  // REDIS_COMMAND_H
