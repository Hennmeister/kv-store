#ifndef KV_STORE_BTREESSTMANAGER_H
#define KV_STORE_BTREESSTMANAGER_H
#include "../Base/SSTManager.h"
#include "../Base/SSTFileManager.h"
#include "BtreeSST.h"

using namespace std;
class BtreeSSTManager: public SSTManager{
private:
    SSTFileManager *fileManager;
    vector<BtreeSST> ssts;
    int total_entries;
    int useBinary;
public:
    ~BtreeSSTManager();
    explicit BtreeSSTManager(SSTFileManager *fileManager, int newFanout, int useBinarySearch);
    bool get(const int& key, int &value) override;
    std::vector<std::pair<int, int>> scan(const int& key1, const int& key2) override;
    bool add_sst(std::vector<std::pair<int, int>> data) override;
};
#endif //KV_STORE_BTREESSTMANAGER_H
