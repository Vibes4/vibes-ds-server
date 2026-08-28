#ifndef REDIS_COMMANDS_PERSISTENCE_COMMANDS_H
#define REDIS_COMMANDS_PERSISTENCE_COMMANDS_H

#include "redis/command.h"

// Persistence commands control writing the dataset to disk. (Mutations are
// already written through on every change, so these mainly force/record a save.)

// SAVE -> "OK". Rewrites the on-disk snapshot synchronously.
class SaveCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// BGSAVE -> "Background saving started". Saves on a background thread.
class BgSaveCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

// LASTSAVE -> the unix timestamp (seconds) of the last successful save.
class LastSaveCommand : public Command
{
public:
    CommandResult execute(StorageEngine &store, const std::vector<std::string> &args) override;
};

#endif  // REDIS_COMMANDS_PERSISTENCE_COMMANDS_H
