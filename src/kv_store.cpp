#include "kv_store.h"

void KVStore::set(const string& key, const string& value) {
    lock_guard<mutex> lock(store_mutex);
    store[key] = value;
    print();
}

string KVStore::get(const string& key) {
    lock_guard<mutex> lock(store_mutex);
    print();
    return store.count(key) ? store[key] : "NULL";
}

void KVStore::del(const string& key) {
    lock_guard<mutex> lock(store_mutex);
    store.erase(key);
    print();
}

void KVStore::print() {
    for (const auto& pair : store) {
        cout << pair.first << ": " << pair.second << endl;
    }
    cout << "Total keys: " << store.size() << endl;
    cout << "------------------------" << endl;
}