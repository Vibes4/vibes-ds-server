#include "store/kv_store.h"

#include <fstream>
#include <iostream>

KVStore::KVStore() {
    // Ensure the persistence file exists before the first read.
    std::ofstream(filename_, std::ios::app).close();
    load_from_disk();
}

void KVStore::load_from_disk() {
    std::ifstream file(filename_);
    if (!file.is_open()) {
        std::cerr << "Warning: could not open " << filename_ << " for reading.\n";
        return;
    }

    std::string key, value;
    while (file >> key >> value) {
        store_[key] = value;
    }
}

void KVStore::persist_to_disk() {
    std::ofstream file(filename_, std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Error: could not open " << filename_ << " for writing.\n";
        return;
    }

    for (const auto& [key, value] : store_) {
        file << key << " " << value << "\n";
    }
}

void KVStore::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    store_[key] = value;
    persist_to_disk();
}

std::string KVStore::get(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto it = store_.find(key);
    return it != store_.end() ? it->second : "(nil)";
}

bool KVStore::del(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (store_.erase(key) == 0) {
        return false;
    }
    persist_to_disk();
    return true;
}

bool KVStore::exists(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    return store_.count(key) > 0;
}

std::unordered_map<std::string, std::string> KVStore::entries() {
    std::lock_guard<std::mutex> lock(mutex_);
    return store_;
}
