#ifndef REDIS_COMMAND_EXECUTOR_H
#define REDIS_COMMAND_EXECUTOR_H

#include "redis/command.h"
#include "store/storage_engine.h"
#include <map>
#include <memory>
#include <string>

// Parses and dispatches Redis-style commands.
//
// It owns the key-value store and a registry mapping command names to the
// controller that handles them. A command line such as "SET name Vaibu" is
// tokenised into a name ("SET") and its arguments ({"name", "Vaibu"}); the
// matching controller is then invoked.
//
// Adding a command means writing one controller class and registering it in the
// constructor; the transport and dispatch layers never change.
class CommandExecutor
{
public:
    CommandExecutor();

    // Tokenises `command_line` on whitespace and runs the matching command.
    // Returns an error result for an empty line or an unknown command.
    CommandResult execute(const std::string &command_line);

private:
    void register_command(const std::string &name, std::unique_ptr<Command> command);

    StorageEngine store_;
    std::map<std::string, std::unique_ptr<Command>> commands_;
};

#endif  // REDIS_COMMAND_EXECUTOR_H
