#include <filesystem>
#include "../../include/SimpleKVStore.h"

void SimpleKVStore::open(const std::string &db_name, Memtable *memt) {
    std::filesystem::create_directory(db_name);
    this->database_name = db_name;
    this->memtable = memt;
}

void SimpleKVStore::put(const int &key, const int &value) {
    memtable->put(key, value);
}

bool SimpleKVStore::get(const int &key, int& value) {
    return memtable->get(key, value);
}

std::vector<std::pair<int, int>> SimpleKVStore::scan(const int &key1, const int &key2) {
    return memtable->scan(key1, key2);
}

void SimpleKVStore::close() {
    memtable->dumpToSst();
}
