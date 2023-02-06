#pragma once

#include <string>
#include <vector>
#include "Memtable.h"

class KVStore {
public:
    // Opens the database with the given name and prepares it to run
    virtual void open(const std::string& database_name, Memtable *memt) =0;

    // Stores a key associated with a value
    virtual void put(const int& key, const int& value) =0;

    // Retrieves a value associated with a given key
    virtual bool get(const int& key, const int& value) =0;

    // Retrieves all KV-pairs in a key range in key order (key1 < key2)
    virtual std::vector<std::pair<int, int>> scan(const int& key1, const int& key2) =0;

    // Closes the database
    virtual void close() =0;
};