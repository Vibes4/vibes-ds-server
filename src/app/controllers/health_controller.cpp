#include "app/controllers/health_controller.h"

#include <string>

namespace
{

std::string about_text()
{
    return "Welcome to the Vibes-Data-Structure Server!\n\n"
           "This server implements a lightweight, Redis-like key-value store over HTTP.\n"
           "It supports basic commands such as:\n"
           "  - SET name Vaibu     -> Store a value\n"
           "  - GET name           -> Retrieve a value\n"
           "  - DEL name           -> Delete a key\n"
           "  - INCR counter       -> Increment a numeric value\n"
           "  - EXPIRE name 60     -> Set a key's time-to-live\n"
           "  - PING               -> Check if the server is running\n\n"
           "Send commands to /redis?cmd=<command line>, e.g.\n"
           "  /redis?cmd=SET+name+Vaibu\n\n"
           "Features:\n"
           "- Uses sockets for efficient communication\n"
           "- Thread-safe with a mutex for concurrent requests\n"
           "- Can handle multiple clients at once\n\n"
           "Developed by: Vaibhav (SDE)\n";
}

} // namespace

void HealthController::register_routes(Router &router)
{
    router.add("/ping", [this](const HttpRequest &request)
               { return handle_ping(request); });
}

HttpResponse HealthController::handle_ping(const HttpRequest & /*request*/)
{
    return HttpResponse::ok(about_text());
}
