#include <filesystem>
#include <iostream>
#include "../../include/SimpleKVStore.h"

void SimpleKVStore::open(const std::string &db_name, Memtable *memt, int maxMemtableSize, SSTManager *sstManager) {
    this->memtable = memt;
    this->sstManager = sstManager;
    this->maxMemtableSize = maxMemtableSize;
}

bool SimpleKVStore::put(const int &key, const int &value) {
    if(memtable->get_size() >= maxMemtableSize){
        if (sstManager->add_sst(memtable->inorderTraversal()))
            return memtable->reset();
        return false;
    }
    return memtable->put(key, value);
}

bool SimpleKVStore::get(const int &key, int& value) {
    if(!memtable->get(key, value)){
        return sstManager->get(key, value);
    }
    return true;
}

std::vector<std::pair<int, int>> SimpleKVStore::scan(const int &key1, const int &key2) {
    return memtable->scan(key1, key2);
}

void SimpleKVStore::close() {
//    memtable->dumpToSst();
}
