#include "../../include/Base/DbOptions.h"
#include "../../include/constants.h"


DbOptions::DbOptions(){
    // Could make into enum of options to guarantee consistency

    // Load default params
    this->memTableType = "RedBlackTree";
    this->maxMemtableSize = PAGE_SIZE;
    this->sstSearch = "BTree";
    this->btreeFanout = 100;
    this->bufferPoolType = "Clock";
    this->bufferPoolMinSize = 1; // MB
    this->bufferPoolMaxSize = 10; // MB
    this->useBinarySearch = 0; // No
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
