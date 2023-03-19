#include "../../include/BTree/BTreeSSTManager.h"
#include "../../include/BTree/BTreeSST.h"
#include "string"
#include "../../include/util.h"

bool BTreeSSTManager::get(const int &key, int &value) {
    for(BTreeSST* sst  : ssts){
        if(sst->get(key, value)){
            return true;
        }
    }
    return false;
}

BTreeSSTManager::BTreeSSTManager(SSTFileManager *fileManager, int newFanout, int useBinarySearch) {
    // TODO: Sort in order depending on filename
    auto files = fileManager->get_files();
    useBinary = useBinarySearch;
    ssts = vector<BTreeSST*>();
    int i = 0;
    for(const pair<string, int>& fileDat : files){
        ssts.push_back(new BTreeSST(fileManager, i, fileDat.first, useBinary));
        total_entries += ssts[i]->getSize();
    }
}

std::vector<std::pair<int, int>> BTreeSSTManager::scan(const int &key1, const int &key2) {
    auto res = vector<pair<int, int>>();

    for (BTreeSST* sst: ssts)
    {
        res = priority_merge(res, sst->scan(key1, key2));
    }
    return res;
}

bool BTreeSSTManager::add_sst(std::vector<std::pair<int, int>> data) {
    auto* new_sst = new BTreeSST(fileManager, ssts.size(), newFanout, data, useBinary);
    ssts.push_back(new_sst);
    total_entries += new_sst->getSize();
}

BTreeSSTManager::~BTreeSSTManager() {
    for (BTreeSST* sst: ssts)
    {
        delete sst;
    }
}
