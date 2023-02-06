﻿#include <filesystem>
#include "../../include/SimpleKVStore.h"
#include "../../include/RedBlackMemtable.h"


void SimpleKVStore::open(const std::string &db_name) {
    std::filesystem::create_directory(db_name);
    this->database_name = db_name;
    this->memtable = new RedBlackMemtable(100, db_name);
}

void SimpleKVStore::put(const int &key, const int &value) {
    memtable->put(key, value);
}

int SimpleKVStore::get(const int &key) {
    return memtable->get(key);
}

std::vector<std::pair<int, int>> SimpleKVStore::scan(const int &key1, const int &key2) {
    return memtable->scan(key1, key2);
}

void SimpleKVStore::close() {
    memtable->dumpToSst();
}
