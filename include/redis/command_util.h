#ifndef REDIS_COMMAND_UTIL_H
#define REDIS_COMMAND_UTIL_H

#include "redis/command.h"
#include <optional>
#include <string>

// Small helpers shared by the command controllers.

// Parses a base-10 integer, requiring the whole string to be consumed.
// Returns nullopt for empty input, junk, or overflow.
inline std::optional<long long> parse_ll(const std::string &text)
{
    if (text.empty())
    {
        return std::nullopt;
    }
    try
    {
        size_t consumed = 0;
        const long long value = std::stoll(text, &consumed);
        if (consumed != text.size())
        {
            return std::nullopt;
        }
        return value;
    }
    catch (...)
    {
        return std::nullopt;
    }
}

// Standard "wrong number of arguments" client error for `command`.
inline CommandResult wrong_args(const std::string &command)
{
    return {false, "wrong number of arguments for '" + command + "'"};
}

#endif  // REDIS_COMMAND_UTIL_H
