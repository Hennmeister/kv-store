//
// Created by Sambamurthy Vijay on 2023-02-06.
//

#ifndef KV_STORE_SST_H
#define KV_STORE_SST_H

#include <string>

class SST{
    virtual bool get(const int& key, int& value) =0;
    virtual std::vector<std::pair<int, int>> scan(const int& key1, const int& key2) =0;
};
#endif //KV_STORE_SST_H