#include "app/controllers/redis_controller.h"

RedisController::RedisController(CommandExecutor &executor) : executor_(executor)
{
}

void RedisController::register_routes(Router &router)
{
    router.add("/redis", [this](const HttpRequest &request)
               { return handle_command(request); });
}

// GET /redis?cmd=<command line>  -> run a Redis-style command.
// The whole command travels in the `cmd` parameter, e.g. cmd=SET+name+Vaibu.
HttpResponse RedisController::handle_command(const HttpRequest &request)
{
    const CommandResult result = executor_.execute(request.param("cmd"));
    return result.ok ? HttpResponse::ok(result.message)
                     : HttpResponse::bad_request(result.message);
}
