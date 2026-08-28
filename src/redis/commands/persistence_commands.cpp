#include "redis/commands/persistence_commands.h"

#include "redis/command_util.h"

#include <thread>

CommandResult SaveCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    (void)args;
    store.save();
    return {true, "OK"};
}

CommandResult BgSaveCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    (void)args;
    // Kick off the save on a detached thread so the client is not blocked.
    // StorageEngine::save() takes the store lock, so this is safe to run concurrently.
    std::thread([&store] { store.save(); }).detach();
    return {true, "Background saving started"};
}

CommandResult LastSaveCommand::execute(StorageEngine &store, const std::vector<std::string> &args)
{
    (void)args;
    return {true, std::to_string(store.last_save_epoch())};
}
