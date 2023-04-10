#include <string>

class DbOptions {
public:
    std::string memTableType; // "RedBlackTree"
    int maxMemtableSize; // in Bytes
    std::string sstSearch; // options: "BTree" "BinarySearh"
    int btreeFanout;
    std::string bufferPoolType; // options: "Clock", "LRU", "None"
    int bufferPoolMinSize; // in MB
    int bufferPoolMaxSize; // in MB
    int useBinarySearch;
    int filter_bits_per_entry;

    DbOptions();
    void setMaxMemtableSize(int maxMemtableSize);
    void setSSTSearch(std::string sstSearch);
    void setBtreeFanout(int btreeFanout);
    void setBufferPoolType(std::string bufferPoolType);
    void setBufferPoolSize(int bufferPoolMinSize, int bufferPoolMaxSize);
};