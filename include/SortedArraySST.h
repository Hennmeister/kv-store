//
// Created by Sambamurthy Vijay on 2023-02-06.
//

#ifndef KV_STORE_SORTEDARRAYSST_H
#define KV_STORE_SORTEDARRAYSST_H

#include "Base/SST.h"
#include <filesystem>

class SortedArraySST: public SST{
private:
    int size;
    std::vector<std::pair<int, int>> data;
public:
    SortedArraySST(const std::filesystem::directory_entry file);
    SortedArraySST(std::string& file, const std::vector<std::pair<int, int>>& data);
    bool get(const int& key, int& value);
    std::vector<std::pair<int, int>> scan(const int& key1, const int& key2);
};

#endif //KV_STORE_SORTEDARRAYSST_H
