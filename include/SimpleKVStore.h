#ifndef KV_STORE_SIMPLEKVSTORE_H
#define KV_STORE_SIMPLEKVSTORE_H

#include "Base/KVStore.h"
#include "Base/Memtable.h"
#include "Base/SSTManager.h"

class SimpleKVStore:public KVStore {
private:
    Memtable *memtable;
    SSTManager *sstManager;
    std::string databaseName;
    int maxMemtableSize;
public:
    // Opens the database with the given name and prepares it to run
    void open(std::string db_path, DbOptions *options = new DbOptions()) override;

    // Stores a key associated with a value
    bool put(const int& key, const int& value) override;

    // Retrieves a value associated with a given key
    bool get(const int& key, int& value) override;

    // Retrieves all KV-pairs in a key range in key order (key1 < key2)
    std::vector<std::pair<int, int>> scan(const int& key1, const int& key2) override;

    // Closes the database
    void close() override;
};
#endif //KV_STORE_SIMPLEKVSTORE_H
