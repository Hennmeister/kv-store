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
};

#endif //KV_STORE_SSTMANAGER_H
