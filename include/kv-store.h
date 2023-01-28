#pragma once

#include <string>
#include <vector>
#include "../src/red-black-tree/red-black-tree.h"

class KVStore {
public:
    // Opens the database with the given name and prepares it to run
    void open(const int& database_name);

    // Stores a key associated with a value
    void put(const int& key, const int& value);

    // Retrieves a value associated with a given key
    int get(const int& key);

    // Retrieves all KV-pairs in a key range in key order (key1 < key2)
    std::vector<std::pair<int, int>> scan(const int& key1, const int& key2);

    // Closes the database
    void close();

private:
    RedBlackTree memtable;

    bool dumpMemtable();
};