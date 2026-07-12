#include "observability/logger.h"
#include "server/server.h"

#include <cstdlib>
#include <string>

namespace
{
// Applies the LOG_LEVEL environment variable (debug|info|warn|error) if set.
// Defaults to Info; use LOG_LEVEL=debug to see per-connection and per-command logs.
void configure_logging()
{
    const char *level = std::getenv("LOG_LEVEL");
    if (level == nullptr)
    {
        return;
    }
    const std::string value = level;
    if (value == "debug")
    {
        Logger::set_level(Logger::Level::Debug);
    }
    else if (value == "info")
    {
        Logger::set_level(Logger::Level::Info);
    }
    else if (value == "warn")
    {
        Logger::set_level(Logger::Level::Warn);
    }
    else if (value == "error")
    {
        Logger::set_level(Logger::Level::Error);
    }
}
} // namespace

int main()
{
    configure_logging();
    Logger::info("server", "vibes-ds-server starting");

    HttpServer server(8080);
    server.start();
    return 0;
}
