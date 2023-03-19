#ifndef KV_STORE_BTREESST_H
#define KV_STORE_BTREESST_H
#include "../Base/SSTFileManager.h"

using namespace std;
class BTreeSST{
private:
    int fanout;
    vector<vector<int>> internal_btree;
    vector<pair<int, int>> internal_data;
    void constructBtree(vector<pair<int, int>> data);
public:
    ~BTreeSST();
    int getSize();
    explicit BTreeSST(SSTFileManager *fileManager, int ind, int fanout, vector<pair<int, int>> data, int useBinarySearch);
    explicit BTreeSST(SSTFileManager *fileManager, int ind, string filename, int useBinarySearch);
    bool get(const int& key, int &value);
    std::vector<std::pair<int, int>> scan(const int& key1, const int& key2);
};
#endif //KV_STORE_BTREESST_H
