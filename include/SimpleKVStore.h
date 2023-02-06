//
// Created by vijay on 6/2/2023.
//

#ifndef KV_STORE_SIMPLEKVSTORE_H
#define KV_STORE_SIMPLEKVSTORE_H

#include "Base/KVStore.h"
#include "Base/Memtable.h"

class SimpleKVStore:public KVStore {
private:
    Memtable *memtable;
    std::string database_name;
public:
    // Opens the database with the given name and prepares it to run
    void open(const std::string& database_name, Memtable *memt) override;

    // Stores a key associated with a value
    void put(const int& key, const int& value) override;

    // Retrieves a value associated with a given key
    int get(const int& key) override;

    // Retrieves all KV-pairs in a key range in key order (key1 < key2)
    std::vector<std::pair<int, int>> scan(const int& key1, const int& key2) override;

    // Closes the database
    void close() override;
};
#endif //KV_STORE_SIMPLEKVSTORE_H
