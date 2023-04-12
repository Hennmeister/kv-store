#include "../../include/Base/DbOptions.h"
#include "../../include/constants.h"
#include "../../include/util.h"


DbOptions::DbOptions(){
    // Load default params

    // Memtable
    this->memTableType = "RedBlackTree";
    this->maxMemtableSize = 10 * MEGABYTE;

    // SST
    this->sstSearch = "BTree";
    this->sstManager = "LSMTree";
    this->btreeFanout = 100;
    this->useBinarySearch = 0; // No

    // Buffer Pool
    this->bufferPoolType = "Clock";
    // this->bufferPoolType = "LRU";
    this->bufferPoolMinSize = 1;
    this->bufferPoolMaxSize = 10;
    this->useBinarySearch = 0;

    // Bloom filter
    this->filterBitsPerEntry = 10;
}

void DbOptions::setMaxMemtableSize(int maxMemtableSize){
    this->maxMemtableSize = maxMemtableSize;
}

void DbOptions::setSSTSearch(std::string sstSearch){
    if (sstSearch == "BinarySearch"){
        this->useBinarySearch = 1;
    } else {
        this->useBinarySearch = 0;
    }
    this->sstSearch = sstSearch;
}

void DbOptions::setSSTManager(std::string sstManager){
    this->sstManager = sstManager;
}

void DbOptions::setBtreeFanout(int btreeFanout){
    this->btreeFanout = btreeFanout;
}

void DbOptions::setBufferPoolType(std::string bufferPoolType){
    this->bufferPoolType = bufferPoolType;
}

void DbOptions::setBufferPoolSize(int bufferPoolMinSize, int bufferPoolMaxSize){
    this->bufferPoolMinSize = bufferPoolMinSize;
    this->bufferPoolMaxSize = bufferPoolMaxSize;
}

void DbOptions::setFilterBitsPerEntry(int filterBitsPerEntry) {
    this->filterBitsPerEntry = filterBitsPerEntry;
}
