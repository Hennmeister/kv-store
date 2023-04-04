#pragma once

#include <string>
#include <vector>
#include "Memtable.h"
#include "SSTManager.h"

class KVStore {
public:
    //TODO: Change from int value type to something else (e.g. an abstract Value interface for various types)

    // Opens the database with the given name and prepares it to run
    virtual void open(const std::string& database_name, int maxMemtableSize) =0;

    // Stores a key associated with a value
    virtual bool put(const int& key, const int& value) =0;

    // Retrieves a value associated with a given key
    virtual bool get(const int& key, int& value) =0;

    // Retrieves all KV-pairs in a key range in key order (key1 < key2)
    virtual std::vector<std::pair<int, int>> scan(const int& key1, const int& key2) =0;

    // Closes the database
    virtual void close() =0;
};