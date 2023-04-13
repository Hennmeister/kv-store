#ifndef KV_STORE_DBOPTIONS_H
#define KV_STORE_DBOPTIONS_H

#include <string>

class DbOptions {
public:
    std::string memTableType; // "RedBlackTree"
    int maxMemtableSize; // in Bytes
    std::string sstManager; // options: "BTreeManager", "LSMTreeManager"
    std::string sstSearch; // options: "BTree", "BinarySearh"
    int btreeFanout;
    std::string bufferPoolType; // options: "Clock", "LRU", "None"
    int bufferPoolMinSize; // in MB
    int bufferPoolMaxSize; // in MB
    int useBinarySearch;
    int filterBitsPerEntry;

    DbOptions();
    void setMaxMemtableSize(int maxMemtableSize);
    void setSSTSearch(std::string sstSearch);
    void setSSTManager(std::string sstManager);
    void setBtreeFanout(int btreeFanout);
    void setBufferPoolType(std::string bufferPoolType);
    void setBufferPoolSize(int bufferPoolMinSize, int bufferPoolMaxSize);
    void setFilterBitsPerEntry(int filterBitsPerEntry);
};

#endif
