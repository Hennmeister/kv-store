﻿#include "../../include/SimpleKVStore.h"
#include "../../include/RedBlackMemtable.h"
#include "../../include/SortedSSTManager.h"
#include "../../include/util.h"
#include <iostream>

void SimpleKVStore::open(const std::string &db_name, int maxMemtableSize)
{
    this->memtable = new RedBlackMemtable();
    this->sstManager = new SortedSSTManager(db_name);
    this->maxMemtableSize = maxMemtableSize;
}

bool SimpleKVStore::put(const int &key, const int &value)
{
    if (memtable->get_size() >= maxMemtableSize)
    {
        if (!sstManager->add_sst(memtable->inorderTraversal()))
            return false;
        memtable->reset();
    }
    return memtable->put(key, value);
}

bool SimpleKVStore::get(const int &key, int &value)
{
    if (!memtable->get(key, value))
    {
        return sstManager->get(key, value);
    }
    return true;
}

std::vector<std::pair<int, int>> SimpleKVStore::scan(const int &key1, const int &key2)
{
    if (key1 > key2)
        return std::vector<std::pair<int, int>>{};

    // TODO: change this to be handled in the algorithm
    if (key1 == key2)
    {
        int val;
        get(key1, val);
        return {(std::pair<int, int>{key1, val})};
    }

    return priority_merge(memtable->scan(key1, key2), sstManager->scan(key1, key2));
}

void SimpleKVStore::close()
{
    // Pad and dump memtable contents to file
    auto dat = memtable->inorderTraversal();
    pad_data(dat, maxMemtableSize);
    sstManager->add_sst(dat);
    delete this->memtable;
    delete this->sstManager;
}

// Delete all data in database, both on disk and in memory
void SimpleKVStore::delete_data() {
    sstManager->delete_data();
    memtable->reset();
}
