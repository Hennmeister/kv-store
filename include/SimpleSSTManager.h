//
// Created by Sambamurthy Vijay on 2023-02-06.
//

#ifndef KV_STORE_SIMPLESSTMANAGER_H
#define KV_STORE_SIMPLESSTMANAGER_H

#include <string>
#include "Base/SSTManager.h"
#include "SortedArraySST.h"

class SimpleSSTManager: public SSTManager{
private:
    std::string directory;
    std::vector<SortedArraySST> SSTs;
public:
    explicit SimpleSSTManager(std::string target_dir);
    bool get(const int& key, int &value) override;
    std::vector<std::pair<int, int>> scan(const int& key1, const int& key2) override;
    bool add_sst(std::vector<std::pair<int, int>> data) override;
};
#endif //KV_STORE_SIMPLESSTMANAGER_H
