#ifndef KV_STORE_SORTEDSSTMANAGER_H
#define KV_STORE_SORTEDSSTMANAGER_H

#include <string>
#include <fstream>
#include "Base/SSTManager.h"
#include "Base/SSTFileManager.h"

using namespace std;
class SortedSSTManager: public SSTManager{
private:
    SSTFileManager *fileManager;
    int total_entries;
    int sst_count;
    std::vector<std::pair<int, int>> get_sst(int sst_ind);
    vector<pair<string, int>> get_ssts();
public:
    ~SortedSSTManager();
    explicit SortedSSTManager(SSTFileManager *fileManager);
    bool get(const int& key, int &value) override;
    std::vector<std::pair<int, int>> scan(const int& key1, const int& key2) override;
    bool add_sst(std::vector<std::pair<int, int>> data) override;
};
#endif //KV_STORE_SORTEDSSTMANAGER_H
