#include "../../include/SimpleKVStore.h"
#include "../../include/RedBlackMemtable.h"
#include "../../include/SortedSSTManager.h"
#include "../../include/util.h"

void SimpleKVStore::open(const std::string &db_name, int maxMemtableSize) {
    this->memtable = new RedBlackMemtable();
    this->sstManager = new SortedSSTManager(db_name);
    this->maxMemtableSize = maxMemtableSize;
}

bool SimpleKVStore::put(const int &key, const int &value) {
    if(memtable->get_size() >= maxMemtableSize) {
        if (!sstManager->add_sst(memtable->inorderTraversal()))
            return false;
        memtable->reset();
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
    return priority_merge(memtable->scan(key1, key2), sstManager->scan(key1, key2));
}

void SimpleKVStore::close() {
    delete this->memtable;
    delete this->sstManager;
}
