#include "store/database.h"

#include <iterator>
#include <utility>

namespace
{

// Matches a Redis-style glob pattern supporting '*' (any run) and '?' (one
// char). Any other character matches literally.
bool glob_match(const std::string &pattern, const std::string &text)
{
    size_t p = 0, t = 0, star = std::string::npos, mark = 0;
    while (t < text.size())
    {
        if (p < pattern.size() && (pattern[p] == '?' || pattern[p] == text[t]))
        {
            ++p;
            ++t;
        }
        else if (p < pattern.size() && pattern[p] == '*')
        {
            star = p++;   // remember the '*' position
            mark = t;     // and where we started skipping
        }
        else if (star != std::string::npos)
        {
            p = star + 1;  // backtrack: let '*' swallow one more char
            t = ++mark;
        }
        else
        {
            return false;
        }
    }
    while (p < pattern.size() && pattern[p] == '*')
    {
        ++p;
    }
    return p == pattern.size();
}

} // namespace

// ---- expiry helpers ------------------------------------------------------

bool Database::is_expired(const Entry &entry) const
{
    return entry.expires_at.has_value() && Clock::now() >= *entry.expires_at;
}

void Database::purge_if_expired(const std::string &key)
{
    const auto it = map_.find(key);
    if (it != map_.end() && is_expired(it->second))
    {
        map_.erase(it);  // lazy expiry does not touch disk, so no dirty flag
        ++lazy_expired_;
    }
}

std::size_t Database::purge_expired()
{
    // Neutral with respect to the access counters: the caller attributes the
    // removals (the sweeper counts them separately from lazy expiry).
    std::size_t removed = 0;
    for (auto it = map_.begin(); it != map_.end();)
    {
        if (is_expired(it->second))
        {
            it = map_.erase(it);
            ++removed;
        }
        else
        {
            ++it;
        }
    }
    return removed;
}

// ---- value access --------------------------------------------------------

std::optional<std::string> Database::get_value(const std::string &key)
{
    purge_if_expired(key);
    const auto it = map_.find(key);
    if (it == map_.end())
    {
        ++misses_;
        return std::nullopt;
    }
    ++hits_;
    return it->second.value;
}

void Database::set_value(const std::string &key, std::string value)
{
    mutations_.push_back({Mutation::Kind::Set, key, value});
    map_[key] = Entry{std::move(value), std::nullopt};  // SET clears any TTL
    dirty_ = true;
}

void Database::set_value_keep_ttl(const std::string &key, std::string value)
{
    purge_if_expired(key);
    mutations_.push_back({Mutation::Kind::Set, key, value});
    const auto it = map_.find(key);
    if (it != map_.end())
    {
        it->second.value = std::move(value);  // keep the existing expiry
    }
    else
    {
        map_[key] = Entry{std::move(value), std::nullopt};
    }
    dirty_ = true;
}

// ---- keyspace ------------------------------------------------------------

bool Database::erase(const std::string &key)
{
    purge_if_expired(key);
    if (map_.erase(key) > 0)
    {
        mutations_.push_back({Mutation::Kind::Delete, key, {}});
        dirty_ = true;
        return true;
    }
    return false;
}

bool Database::contains(const std::string &key)
{
    purge_if_expired(key);
    const bool found = map_.count(key) > 0;
    if (found)
    {
        ++hits_;
    }
    else
    {
        ++misses_;
    }
    return found;
}

bool Database::rename(const std::string &from, const std::string &to)
{
    purge_if_expired(from);
    const auto it = map_.find(from);
    if (it == map_.end())
    {
        return false;
    }
    // Copy the entry out before touching the map, then move it under the new
    // key. Doing the erase first keeps RENAME onto the same name a no-op.
    Entry entry = it->second;  // preserves value and any TTL
    map_.erase(from);
    const std::string value = entry.value;
    map_[to] = std::move(entry);

    mutations_.push_back({Mutation::Kind::Delete, from, {}});
    mutations_.push_back({Mutation::Kind::Set, to, value});
    dirty_ = true;
    return true;
}

std::vector<std::string> Database::keys(const std::string &pattern)
{
    lazy_expired_ += purge_expired();
    std::vector<std::string> matches;
    for (const auto &[key, entry] : map_)
    {
        (void)entry;
        if (glob_match(pattern, key))
        {
            matches.push_back(key);
        }
    }
    return matches;
}

std::optional<std::string> Database::random_key()
{
    lazy_expired_ += purge_expired();
    if (map_.empty())
    {
        return std::nullopt;
    }
    return map_.begin()->first;  // "random" = first key in hash order
}

size_t Database::size()
{
    lazy_expired_ += purge_expired();
    return map_.size();
}

void Database::clear()
{
    map_.clear();
    mutations_.push_back({Mutation::Kind::Clear, {}, {}});
    dirty_ = true;
}

// ---- expiration ----------------------------------------------------------

bool Database::set_expiry_ms(const std::string &key, long long ttl_ms)
{
    purge_if_expired(key);
    const auto it = map_.find(key);
    if (it == map_.end())
    {
        return false;
    }
    it->second.expires_at = Clock::now() + std::chrono::milliseconds(ttl_ms);
    return true;  // expiry lives only in memory, so nothing to persist
}

long long Database::pttl_ms(const std::string &key)
{
    purge_if_expired(key);
    const auto it = map_.find(key);
    if (it == map_.end())
    {
        return -2;  // no such key
    }
    if (!it->second.expires_at.has_value())
    {
        return -1;  // key exists but has no expiry
    }
    const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
        *it->second.expires_at - Clock::now());
    return remaining.count() > 0 ? remaining.count() : 0;
}

bool Database::clear_expiry(const std::string &key)
{
    purge_if_expired(key);
    const auto it = map_.find(key);
    if (it == map_.end() || !it->second.expires_at.has_value())
    {
        return false;
    }
    it->second.expires_at.reset();
    return true;
}

// ---- persistence support -------------------------------------------------

std::unordered_map<std::string, std::string> Database::snapshot()
{
    lazy_expired_ += purge_expired();
    std::unordered_map<std::string, std::string> out;
    out.reserve(map_.size());
    for (const auto &[key, entry] : map_)
    {
        out[key] = entry.value;
    }
    return out;
}

void Database::load_raw(const std::string &key, std::string value)
{
    map_[key] = Entry{std::move(value), std::nullopt};
}

bool Database::take_dirty()
{
    const bool was_dirty = dirty_;
    dirty_ = false;
    return was_dirty;
}

std::vector<Database::Mutation> Database::take_mutations()
{
    std::vector<Mutation> drained;
    drained.swap(mutations_);
    return drained;
}

// ---- observability -------------------------------------------------------

Database::KeyspaceStats Database::keyspace_stats() const
{
    return {hits_, misses_, lazy_expired_};
}

std::size_t Database::approx_bytes() const
{
    std::size_t total = 0;
    for (const auto &[key, entry] : map_)
    {
        if (!is_expired(entry))
        {
            total += key.size() + entry.value.size();
        }
    }
    return total;
}
