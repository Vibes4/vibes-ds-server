#include "kv_store.h"

// Constructor: Load data from file when server starts
KVStore::KVStore() {
    loadFromDisk();
}

// Persist key-value pair to file (append mode)
void KVStore::persistToDisk(const string& key, const string& value) {
    // Remove the existing key from the file before adding the new one
    removeFromDisk(key);
    ofstream file(FILENAME, ios::app);
    if (!file.is_open()) {
        cerr << "Error: Could not open file for writing: " << FILENAME << endl;
        return;
    }
    file << key << " " << value << endl;
    file.close();
}

// Remove a key from the file (rewrite without it)
void KVStore::removeFromDisk(const string& key) {
    ifstream file(FILENAME);
    if (!file.is_open()) {
        cerr << "Error: Could not open file for reading: " << FILENAME << endl;
        return;
    }

    ofstream temp("temp.db");
    if (!temp.is_open()) {
        cerr << "Error: Could not open temporary file for writing" << endl;
        return;
    }

    string k, v;
    while (file >> k >> v) {
        if (k != key) {
            temp << k << " " << v << endl;
        }
    }

    file.close();
    temp.close();

    // Ensure proper conversion to C-style string using .c_str()
    remove(FILENAME.c_str());
    rename("temp.db", FILENAME.c_str());
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
    persistToDisk(key, value);
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
    removeFromDisk(key);
    print();
}

void KVStore::print() {
    for (const auto& pair : store) {
        cout << pair.first << ": " << pair.second << endl;
    }
    cout << "Total keys: " << store.size() << endl;
    cout << "------------------------" << endl;
}