#ifndef APP_CONTROLLERS_REDIS_CONTROLLER_H
#define APP_CONTROLLERS_REDIS_CONTROLLER_H

#include "http/router.h"
#include "redis/command_executor.h"

// Controller for the /redis endpoint.
//
// It is the thin HTTP adapter for the command layer: it pulls the command line
// out of the `cmd` query parameter, hands it to the CommandExecutor, and turns
// the CommandResult into an HttpResponse. No command or storage logic lives here.
class RedisController
{
public:
    explicit RedisController(CommandExecutor &executor);

    // Registers this controller's routes on the given router.
    void register_routes(Router &router);

private:
    HttpResponse handle_command(const HttpRequest &request);

    CommandExecutor &executor_;
};

#endif  // APP_CONTROLLERS_REDIS_CONTROLLER_H
