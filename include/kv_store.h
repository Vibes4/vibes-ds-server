#ifndef KV_STORE_H
#define KV_STORE_H


#include <unordered_map>
#include <string>
#include <mutex>
using namespace std;

class KVStore {
    private:
        unordered_map<string, string> store;
        mutex store_mutex; // Ensures thread safety
    
    public:
        void set(const string& key, const string& value);
        string get(const string& key);
        void del(const string& key);
};
    
#endif // KV_STORE_H