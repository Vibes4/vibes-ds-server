#ifndef STORE_DATABASE_H
#define STORE_DATABASE_H

#include <chrono>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

// The in-memory keyspace: a string-to-string map with per-key expiration.
//
// Database provides only the low-level *mechanics* of a keyspace (lookup,
// storage, deletion, key enumeration, TTL bookkeeping). It knows nothing about
// individual commands and, deliberately, nothing about threading or disk I/O --
// that is StorageEngine's job. It is NOT thread-safe on its own; callers reach it
// through StorageEngine::with_lock, which holds the lock for the whole operation.
//
// Value *semantics* (integer math for INCR, concatenation for APPEND, the
// return value of GETSET, ...) live in the command controllers, which compose
// these primitives.
class Database
{
public:
    // ---- value access ----
    // Returns the value, or nullopt if the key is absent or expired.
    std::optional<std::string> get_value(const std::string &key);
    // Stores a value, clearing any existing TTL (Redis SET semantics).
    void set_value(const std::string &key, std::string value);
    // Stores a value but preserves an existing key's TTL (INCR/APPEND semantics).
    void set_value_keep_ttl(const std::string &key, std::string value);

    // ---- keyspace ----
    bool erase(const std::string &key);        // true if a key was removed
    bool contains(const std::string &key);
    bool rename(const std::string &from, const std::string &to);  // false if missing
    std::vector<std::string> keys(const std::string &pattern);    // glob: * and ?
    std::optional<std::string> random_key();
    size_t size();
    void clear();

    // ---- expiration (milliseconds) ----
    bool set_expiry_ms(const std::string &key, long long ttl_ms);  // false if missing
    long long pttl_ms(const std::string &key);  // -2 missing, -1 no TTL, else remaining
    bool clear_expiry(const std::string &key);   // false if none/missing

    // ---- maintenance & persistence support (used by StorageEngine) ----
    void purge_expired();                            // drop every expired key
    std::unordered_map<std::string, std::string> snapshot();  // live key/value copy
    void load_raw(const std::string &key, std::string value); // insert without a TTL
    bool take_dirty();  // returns whether the data changed since the last call, and resets

private:
    using Clock = std::chrono::steady_clock;

    struct Entry
    {
        std::string value;
        std::optional<Clock::time_point> expires_at;  // absent = never expires
    };

    bool is_expired(const Entry &entry) const;
    void purge_if_expired(const std::string &key);

    std::unordered_map<std::string, Entry> map_;
    bool dirty_ = false;  // set when on-disk content needs rewriting
};

#endif  // STORE_DATABASE_H
