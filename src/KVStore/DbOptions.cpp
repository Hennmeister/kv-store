#include "../../include/Base/DbOptions.h"
#include "../../include/constants.h"


DbOptions::DbOptions(){
    // Could make into enum of options to guarantee consistency

    // Load default params
    this->memTableType = "RedBlackTree";
    this->maxMemtableSize = PAGE_SIZE * 2;
    this->sstSearch = "BTree";
    this->btreeFanout = 5;
    this->bufferPoolType = "LRU";
    this->bufferPoolMinSize = 1;
    this->bufferPoolMaxSize = 10;
    this->useBinarySearch = 0;
    this->filterBitsPerEntry = 10;
}

void DbOptions::setMaxMemtableSize(int maxMemtableSize){
    this->maxMemtableSize = maxMemtableSize;
}

void DbOptions::setSSTSearch(std::string sstSearch){
    if (sstSearch == "BinarySearch"){
        this->useBinarySearch = 1;
    }
    this->sstSearch = sstSearch;
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
