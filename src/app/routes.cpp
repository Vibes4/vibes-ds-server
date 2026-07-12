#include "app/routes.h"

#include <string>

namespace
{

std::string about_text()
{
    return "Welcome to the Vibes-Data-Structure Server!\n\n"
           "This server implements a lightweight, Redis-like key-value store over HTTP.\n"
           "It supports basic commands such as:\n"
           "  - SET <key> <value>  -> Store a value\n"
           "  - GET <key>          -> Retrieve a value\n"
           "  - DEL <key>          -> Delete a key\n"
           "  - EXISTS <key>       -> Check whether a key exists\n"
           "  - PING               -> Check if the server is running\n\n"
           "Features:\n"
           "- Uses sockets for efficient communication\n"
           "- Thread-safe with a mutex for concurrent requests\n"
           "- Can handle multiple clients at once\n\n"
           "Developed by: Vaibhav (SDE)\n";
}

// GET /redis?cmd=SET&key=...&value=...  -> run a Redis-style command.
HttpResponse handle_redis_command(RedisService &redis, const HttpRequest &request)
{
    const CommandResult result = redis.execute(
        request.param("cmd"), request.param("key"), request.param("value"));
    return result.ok ? HttpResponse::ok(result.message)
                     : HttpResponse::bad_request(result.message);
}

} // namespace

void register_routes(Router &router, RedisService &redis)
{
    router.add("/redis", [&redis](const HttpRequest &req)
               { return handle_redis_command(redis, req); });
    router.add("/ping", [](const HttpRequest &)
               { return HttpResponse::ok(about_text()); });
}
