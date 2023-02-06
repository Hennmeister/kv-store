#ifndef KV_STORE_MEMTABLE_DATA_H
#define KV_STORE_MEMTABLE_DATA_H

#include <tuple>
#include <string>
#include <vector>

class MemtableData {
public:
    virtual void put(const int& key, const int& value) = 0;
    virtual int get(const int& key) = 0;
    virtual std::vector<std::pair<int, int>> scan(const int& key1, const int& key2) = 0;
    virtual std::vector<std::pair<int, int>> inorderTraversal() = 0;
    virtual int getSize() = 0;
};

#endif //KV_STORE_MEMTABLE_DATA_H
