#ifndef OBSERVABILITY_LOGGER_H
#define OBSERVABILITY_LOGGER_H

#include <atomic>
#include <mutex>
#include <string>

// A tiny, thread-safe, centralized logger.
//
// All log output in the project goes through these static methods, so the
// format, destination, and minimum level are controlled in one place. Info and
// Debug go to stdout; Warn and Error go to stderr. Each line looks like:
//   [YYYY-MM-DD HH:MM:SS] [LEVEL] [component] message
//
// The minimum level defaults to Info; set it to Debug (e.g. LOG_LEVEL=debug) to
// see per-connection and per-command detail.
class Logger
{
public:
    enum class Level
    {
        Debug = 0,
        Info = 1,
        Warn = 2,
        Error = 3,
    };

    static void set_level(Level level);
    static Level level();

    static void debug(const std::string &component, const std::string &message);
    static void info(const std::string &component, const std::string &message);
    static void warn(const std::string &component, const std::string &message);
    static void error(const std::string &component, const std::string &message);

private:
    static void log(Level level, const std::string &component, const std::string &message);

    static std::mutex mutex_;
    static std::atomic<Level> min_level_;
};

#endif  // OBSERVABILITY_LOGGER_H
