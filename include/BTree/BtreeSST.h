#ifndef KV_STORE_BTREESST_H
#define KV_STORE_BTREESST_H
#include "../Base/SSTFileManager.h"

using namespace std;
class BtreeSST{
public:
    ~BtreeSST();
    explicit BtreeSST(SSTFileManager *fileManager, int ind, int fanout, int useBinarySearch);
    explicit BtreeSST(SSTFileManager *fileManager, int ind, string filename, int useBinarySearch);
    bool get(const int& key, int &value);
    std::vector<std::pair<int, int>> scan(const int& key1, const int& key2);
};
#endif //KV_STORE_BTREESST_H
