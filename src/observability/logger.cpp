#include "observability/logger.h"

#include <chrono>
#include <ctime>
#include <iostream>

std::mutex Logger::mutex_;
std::atomic<Logger::Level> Logger::min_level_{Logger::Level::Info};

namespace
{

const char *level_name(Logger::Level level)
{
    switch (level)
    {
    case Logger::Level::Debug:
        return "DEBUG";
    case Logger::Level::Info:
        return "INFO";
    case Logger::Level::Warn:
        return "WARN";
    case Logger::Level::Error:
        return "ERROR";
    }
    return "?";
}

std::string now_timestamp()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    return buffer;
}

} // namespace

void Logger::set_level(Level level)
{
    min_level_.store(level);
}

Logger::Level Logger::level()
{
    return min_level_.load();
}

void Logger::debug(const std::string &component, const std::string &message)
{
    log(Level::Debug, component, message);
}

void Logger::info(const std::string &component, const std::string &message)
{
    log(Level::Info, component, message);
}

void Logger::warn(const std::string &component, const std::string &message)
{
    log(Level::Warn, component, message);
}

void Logger::error(const std::string &component, const std::string &message)
{
    log(Level::Error, component, message);
}

void Logger::log(Level level, const std::string &component, const std::string &message)
{
    if (static_cast<int>(level) < static_cast<int>(min_level_.load()))
    {
        return;
    }
    std::ostream &out =
        static_cast<int>(level) >= static_cast<int>(Level::Warn) ? std::cerr : std::cout;

    std::lock_guard<std::mutex> lock(mutex_);
    // Flush every line so logs appear promptly and survive a crash, even when
    // stdout/stderr is redirected to a file (where it would otherwise be
    // block-buffered).
    out << "[" << now_timestamp() << "] [" << level_name(level) << "] ["
        << component << "] " << message << std::endl;
}
