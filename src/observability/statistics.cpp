#include "observability/statistics.h"

namespace
{
long long to_epoch_seconds(std::chrono::system_clock::time_point tp)
{
    return std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
}
} // namespace

Statistics::Statistics()
    : start_wall_(std::chrono::system_clock::now()),
      start_steady_(std::chrono::steady_clock::now())
{
}

void Statistics::record_command(std::chrono::microseconds latency)
{
    commands_.fetch_add(1, std::memory_order_relaxed);
    total_latency_us_.fetch_add(static_cast<std::uint64_t>(latency.count()),
                                std::memory_order_relaxed);
}

void Statistics::record_read()
{
    reads_.fetch_add(1, std::memory_order_relaxed);
}

void Statistics::record_write()
{
    writes_.fetch_add(1, std::memory_order_relaxed);
}

void Statistics::record_persistence()
{
    persistence_ops_.fetch_add(1, std::memory_order_relaxed);
    last_persistence_epoch_.store(to_epoch_seconds(std::chrono::system_clock::now()),
                                  std::memory_order_relaxed);
}

void Statistics::record_sweeper_expired(std::uint64_t count)
{
    sweeper_expired_.fetch_add(count, std::memory_order_relaxed);
}

std::uint64_t Statistics::commands() const { return commands_.load(); }
std::uint64_t Statistics::reads() const { return reads_.load(); }
std::uint64_t Statistics::writes() const { return writes_.load(); }
std::uint64_t Statistics::persistence_ops() const { return persistence_ops_.load(); }
std::uint64_t Statistics::sweeper_expired() const { return sweeper_expired_.load(); }

double Statistics::average_latency_us() const
{
    const std::uint64_t count = commands_.load();
    if (count == 0)
    {
        return 0.0;
    }
    return static_cast<double>(total_latency_us_.load()) / static_cast<double>(count);
}

long long Statistics::start_epoch() const
{
    return to_epoch_seconds(start_wall_);
}

long long Statistics::uptime_seconds() const
{
    return std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::steady_clock::now() - start_steady_)
        .count();
}

long long Statistics::last_persistence_epoch() const
{
    return last_persistence_epoch_.load();
}
