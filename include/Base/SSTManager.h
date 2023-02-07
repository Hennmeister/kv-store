//
// Created by Sambamurthy Vijay on 2023-02-06.
//

#ifndef KV_STORE_SSTMANAGER_H
#define KV_STORE_SSTMANAGER_H
#include <vector>

class SSTManager{
public:
    virtual int open(std::string& directory) =0;
    virtual bool get(const int& key, int& value) =0;
    virtual std::vector<std::pair<int, int>> scan(const int& key1, const int& key2) =0;
    virtual bool add_sst(std::vector<std::pair<int, int>> data) =0;
};

#endif //KV_STORE_SSTMANAGER_H