#ifndef KV_STORE_H
#define KV_STORE_H

#include <unordered_map>
#include <string>
#include <mutex>
#include <iostream>
#include <fstream>

using namespace std;

class KVStore {
private:
    unordered_map<string, string> store;
    mutex store_mutex; // Ensures thread safety

    const string FILENAME = "data.db"; // File for persistence

    void persistToDisk(const string& key, const string& value);
    void removeFromDisk(const string& key);
    void loadFromDisk();

public:
    KVStore();
    void set(const string& key, const string& value);
    string get(const string& key);
    unordered_map<string, string> getAll();
    void del(const string& key);
    void print();
};

#endif // KV_STORE_H