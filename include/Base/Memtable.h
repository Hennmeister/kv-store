//
// Created by vijay on 6/2/2023.
//

#ifndef KV_STORE_MEMTABLE_H
#define KV_STORE_MEMTABLE_H
#include <vector>

class Memtable {
public:
    virtual int get_size() =0;
    virtual bool put(const int& key, const int& value) =0;
    virtual bool get(const int& key, int& value) =0;
    virtual std::vector<std::pair<int, int>> scan(const int& key1, const int& key2) =0;
    virtual std::vector<std::pair<int, int>> inorderTraversal() =0;
    virtual bool reset() =0;
};



#endif //KV_STORE_MEMTABLE_H
