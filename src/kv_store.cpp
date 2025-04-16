#include "kv_store.h"

// Constructor: Load data from file when server starts
KVStore::KVStore() {
    // Ensure the data file exists; create it if it doesn't
    ofstream file(FILENAME, ios::app);
    if (!file.is_open()) {
        cerr << "Error: Could not create file: " << FILENAME << endl;
    }
    file.close();
    loadFromDisk();
}

// Persist key-value pair to file (append mode)
void KVStore::persistToDisk() {
    ofstream file(FILENAME);
    if (!file.is_open()) {
        cerr << "Error: Could not open file for writing: " << FILENAME << endl;
        return;
    }

    for (const auto& [key, value] : store) {
        file << key << " " << value << endl;
    }

    file.close();
}

// Load all key-value pairs from disk into memory
void KVStore::loadFromDisk() {
    ifstream file(FILENAME);
    if (!file.is_open()) {
        cerr << "Error: Could not open file for reading: " << FILENAME << endl;
        return;
    }

    string key, value;
    while (file >> key >> value) {
        store[key] = value;
    }
    file.close();
}

void KVStore::set(const string& key, const string& value) {
    lock_guard<mutex> lock(store_mutex);
    store[key] = value;
    persistToDisk();
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
    persistToDisk();
    print();
}

void KVStore::print() {
    for (const auto& pair : store) {
        cout << pair.first << ": " << pair.second << endl;
    }
    cout << "Total keys: " << store.size() << endl;
    cout << "------------------------" << endl;
}

unordered_map<string, string> KVStore::getAll() {
    return store;
}