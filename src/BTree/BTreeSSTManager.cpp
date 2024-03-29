#include "../../include/BTree/BTreeSSTManager.h"
#include "../../include/BTree/BTreeSST.h"
#include "../../include/util.h"
#include "../../include/constants.h"
#include <string>

using namespace std;

bool BTreeSSTManager::get(const int &key, int &value) {
    for(BTreeSST* sst  : ssts){
        if(sst->get(key, value)){
            return true;
        }
    }
    return false;
}

BTreeSSTManager::BTreeSSTManager(SSTFileManager *fileManager, int newFanout, int useBinarySearch, int filter_bits_per_entry) {
    auto files = fileManager->get_files();
    // Reverse SST order by filename so that newer SSTs are first.
    std::sort(files.begin(), files.end(), sortByFname);
    std::reverse(files.begin(), files.end());

    this->useBinary = useBinarySearch;
    this->filter_bits_per_entry = filter_bits_per_entry;

    this->fileManager = fileManager;
    this->newFanout = newFanout;
    ssts = vector<BTreeSST*>();

    int i = 0;
    for(const pair<string, int>& fileDat : files){
        ssts.push_back(new BTreeSST(fileManager, fileDat.first, fileDat.second, useBinary));
        total_entries += ssts[i]->getSize();
        i++;
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
    if (data.size() % PAGE_NUM_ENTRIES != 0)
        return false;
    auto* new_sst = new BTreeSST(fileManager, ssts.size() - 1, newFanout, data, useBinary, filter_bits_per_entry);
    ssts.insert(ssts.begin(),new_sst);
    total_entries += new_sst->getSize();
    return true;
}

BTreeSSTManager::~BTreeSSTManager() {
    // Free memory
    for (BTreeSST* sst: ssts)
    {
        delete sst;
    }
}
