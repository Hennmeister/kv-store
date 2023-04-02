//
// Created by Sambamurthy Vijay on 2023-02-06.
//

#ifndef KV_STORE_SSTMANAGER_H
#define KV_STORE_SSTMANAGER_H
#include <vector>

class SSTManager{
public:
    //TODO: Change from int value type to something else (e.g. an abstract Value interface for various types)
    virtual bool get(const int& key, int& value) =0;
    virtual std::vector<std::pair<int, int>> scan(const int& key1, const int& key2) =0;
    virtual bool add_sst(std::vector<std::pair<int, int>> data) =0;
    virtual ~SSTManager() = default;
    virtual void delete_data() =0;
};

#endif //KV_STORE_SSTMANAGER_H
