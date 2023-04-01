#ifndef KV_STORE_BTREESSTMANAGER_H
#define KV_STORE_BTREESSTMANAGER_H
#include "../Base/SSTManager.h"
#include "../Base/SSTFileManager.h"
#include "BTreeSST.h"

using namespace std;
class BTreeSSTManager: public SSTManager{
private:
    SSTFileManager *fileManager;
    vector<BTreeSST*> ssts;
    int total_entries;
    int useBinary;
    int newFanout;
public:
    ~BTreeSSTManager();
    explicit BTreeSSTManager(SSTFileManager *fileManager, int newFanout, int useBinarySearch);
    bool get(const int& key, int &value) override;
    std::vector<std::pair<int, int>> scan(const int& key1, const int& key2) override;
    bool add_sst(std::vector<std::pair<int, int>> data) override;
};
#endif //KV_STORE_BTREESSTMANAGER_H
