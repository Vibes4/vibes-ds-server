#include "kv_store.h"

void KVStore::set(const string& key, const string& value) {
    lock_guard<mutex> lock(store_mutex);
    store[key] = value;
}

string KVStore::get(const string& key) {
    lock_guard<mutex> lock(store_mutex);
    return store.count(key) ? store[key] : "NULL";
}

void KVStore::del(const string& key) {
    lock_guard<mutex> lock(store_mutex);
    store.erase(key);
}
