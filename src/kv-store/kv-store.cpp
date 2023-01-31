#include <filesystem>
#include "../../include/kv-store.h"

void KVStore::open(const std::string &database_name) {
    std::filesystem::create_directory(database_name);
    this->database_name = database_name;
}

KVStore::KVStore() {
    this->database_name = "";
    memtable = new RedBlackTree(10,database_name);
}

void KVStore::put(const int &key, const int &value) {
    memtable->insert(key, value);
}

int KVStore::get(const int &key) {
    return memtable->get(key);
}

std::vector<std::pair<int, int>> KVStore::scan(const int &key1, const int &key2) {
    return memtable->scan(key1, key2);
}

void KVStore::close() {

}

