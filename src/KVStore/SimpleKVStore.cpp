#include "../../include/SimpleKVStore.h"
#include "../../include/RedBlack/RedBlackMemtable.h"
#include "../../include/LSMTreeManager.h"
#include "../../include/BTree/BTreeSSTManager.h"
#include "../../include/util.h"
#include "../../include/SimpleSSTFileManager.h"
#include "../../include/Base/BufferPool.h"
#include "../../include/BufferPool/LRUBuffer/LRUBuffer.h"
#include "../../include/BufferPool/ClockBuffer/ClockBuffer.h"
#include <iostream>

void SimpleKVStore::open(std::string db_path, DbOptions *options)
{
    this->memtable = new RedBlackMemtable();

    // defaults to no buffer (buffer of size 0)
    BufferPool *bufferPool = new LRUBuffer(0, 0);

    if (options->bufferPoolType == "LRU")
        bufferPool = new LRUBuffer(options->bufferPoolMinSize, options->bufferPoolMaxSize);
    else if (options->bufferPoolType == "Clock") {
        bufferPool = new ClockBuffer(options->bufferPoolMinSize, options->bufferPoolMaxSize);
    }
    this->fileManager = new SimpleSSTFileManager(db_path, bufferPool);

    this->sstManager = new LSMTreeManager(this->fileManager,
                                          options->btreeFanout,
                                          options->useBinarySearch,
                                          options->maxMemtableSize,
                                          options->filterBitsPerEntry);

    if (options->sstManager == "BTree") {
        this->sstManager = new BTreeSSTManager(this->fileManager, options->btreeFanout, options->useBinarySearch,options->filterBitsPerEntry);
    }

    this->maxMemtableSize = options->maxMemtableSize;

}

bool SimpleKVStore::put(const int &key, const int &value)
{
    int discard;
    if (!memtable->get(key, discard) && memtable->get_size() >= maxMemtableSize)
    {
        if (!this->sstManager->add_sst(this->memtable->inorderTraversal()))
            return false;
        this->memtable->reset();
    }
    return this->memtable->put(key, value);
}

bool SimpleKVStore::get(const int &key, int &value)
{
    if (!this->memtable->get(key, value))
        return this->sstManager->get(key, value);
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

    return priority_merge(this->memtable->scan(key1, key2), this->sstManager->scan(key1, key2));
}

void SimpleKVStore::close()
{
    auto dat = memtable->inorderTraversal();
    sstManager->add_sst(dat);
    delete this->memtable;
    delete this->sstManager;
}

void SimpleKVStore::set_buffer_pool_max_size(const int &new_max) {
    this->fileManager->cache->set_max_size(new_max);
}
