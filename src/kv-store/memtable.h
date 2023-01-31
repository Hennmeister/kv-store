#ifndef KV_STORE_MEMTABLE_H
#define KV_STORE_MEMTABLE_H

#include <tuple>

class Memtable {
private:
    int capacity;
    int size;
    std::string directory;
public:
    virtual void insert(const int& key, const int& value) = 0;
    virtual int get(const int& key) = 0;
    virtual std::vector<std::pair<int, int>> scan(const int& key1, const int& key2) = 0;
    virtual std::vector<std::pair<int, int>> inorderTraversal() = 0;
};

#endif //KV_STORE_MEMTABLE_H
