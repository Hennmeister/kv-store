#include "../include/LSMTreeManager.h"
#include "string"
#include "../include/util.h"
#include "../include/constants.h"
#include <math.h>

bool LSMTreeManager::get(const int &key, int &value) {
    for (auto level: levels)
    {
        for(auto sst: level) {
            if (sst->get(key, value)) {
                return true;
            }
        }
    }
    return false;
}

bool sortByFname(const pair<string,int> &a,
                 const pair<string,int> &b)
{
    int pos_a = stoi(a.first.substr(0, a.first.size()-4));
    int pos_b = stoi(b.first.substr(0, b.first.size()-4));
    return (pos_a < pos_b);
}

LSMTreeManager::LSMTreeManager(SSTFileManager *fileManager, int newFanout, int useBinarySearch, int memtable_size) {
    auto files = fileManager->get_files();
    // Reverse SST order by filename so that newer SSTs are first.
    std::sort(files.begin(), files.end(), sortByFname);
    std::reverse(files.begin(), files.end());

    this->useBinary = useBinarySearch;

    this->fileManager = fileManager;
    this->sst_counter = 0;
    this->newFanout = newFanout;
    this->levels = vector<vector<BTreeSST*>>();
    this->levels.push_back(vector<BTreeSST*>());

    auto tmp_ssts = vector<BTreeSST *>();
    int i = 0;
    for(const pair<string, int>& fileDat : files){
        tmp_ssts.push_back(new BTreeSST(fileManager, fileDat.first, fileDat.second, useBinary));
        total_entries += tmp_ssts[i]->getSize();
        int file_num = stoi(fileDat.first.substr(0, fileDat.first.find('.')));
        if(file_num -1 > sst_counter){
            sst_counter = file_num - 1;
        }
        i++;
    }


    int start_log = log2(memtable_size);
    for(BTreeSST* sst: tmp_ssts){
        int level = ceil(log2((double) sst->getSize())) - start_log;
        while(this->levels.size() <= level){
            this->levels.push_back(vector<BTreeSST*>());
        }
        this->levels[level].push_back(sst);
    }
}

std::vector<std::pair<int, int>> LSMTreeManager::scan(const int &key1, const int &key2) {
    auto res = vector<pair<int, int>>();

    for (auto level: levels)
    {
        for(auto sst: level) {
            res = priority_merge(res, sst->scan(key1, key2));
        }
    }
    return res;
}

bool LSMTreeManager::add_sst(std::vector<std::pair<int, int>> data) {
    if (data.size() % PAGE_NUM_ENTRIES != 0)
        return false;
    auto* new_sst = new BTreeSST(fileManager, sst_counter, newFanout, data, useBinary);
    sst_counter++;
    levels[0].push_back(new_sst);
    total_entries += new_sst->getSize();
    if(levels[0].size() == 2){
        return this->compact_tree(0);
    }
    return true;
}

LSMTreeManager::~LSMTreeManager() {
    // Free memory
//    for (BTreeSST* sst: ssts)
//    {
//        delete sst;
//    }
}


BTreeSST* LSMTreeManager::combine_SST(BTreeSST* newer, BTreeSST* older){
    auto data_newer = newer->scan(INT_MIN, INT_MAX);
    auto data_older = older->scan(INT_MIN, INT_MAX);

    auto result_data = priority_merge(data_newer, data_older);
    return new BTreeSST(fileManager, sst_counter, newFanout, result_data, useBinary);
}

bool LSMTreeManager::compact_tree(int level) {
    auto res = combine_SST(levels[level][1], levels[level][0]);
    sst_counter++;
    delete levels[level][0];
    delete levels[level][1];
    levels[level].clear();
    if(levels.size() == level + 1){
        auto new_level = vector<BTreeSST*>();
        new_level.push_back(res);
        levels.push_back(new_level);
    }else{
        levels[level+1].push_back(res);
        if(levels[level+1].size() == 2){
            return compact_tree(level + 1);
        }
    }
    return true;
}
