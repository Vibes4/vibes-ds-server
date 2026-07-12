#ifndef STORE_KV_STORE_H
#define STORE_KV_STORE_H

#include <mutex>
#include <string>
#include <unordered_map>

// A thread-safe, string-to-string key-value store with simple file-backed
// persistence. Every mutation is written through to disk, and the store is
// reloaded from disk on construction so data survives restarts.
class KVStore {
public:
    KVStore();

    void set(const std::string& key, const std::string& value);
    std::string get(const std::string& key);        // "(nil)" if absent
    bool del(const std::string& key);                // true if a key was removed
    bool exists(const std::string& key);
    std::unordered_map<std::string, std::string> entries();  // snapshot copy

private:
    void load_from_disk();
    void persist_to_disk();  // caller must hold mutex_

    std::unordered_map<std::string, std::string> store_;
    std::mutex mutex_;  // guards store_ and the persistence file
    const std::string filename_ = "cache/key-value.db";
};

#endif  // STORE_KV_STORE_H
