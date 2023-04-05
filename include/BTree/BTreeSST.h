#ifndef KV_STORE_BTREESST_H
#define KV_STORE_BTREESST_H
#include "../Base/SSTFileManager.h"
#include "string"

using namespace std;
class BTreeSST{
private:
    int fanout;
    int size;
    string filename;
    SSTFileManager *fileManager;
    int useBinary;
    int internal_node_pages;
    void constructBtree(const vector<pair<int, int>>& data);
    int binary_scan(int key);
    int binary_lower_bound(int key);
public:
    vector<vector<int>> internal_btree;
    ~BTreeSST();
    int getSize() const;
    int get_internal_node_count();
    vector<pair<int, int>> get_pages(int start_ind, int end_ind);
    vector<pair<int, int>> get_page(int page_ind);
    explicit BTreeSST(SSTFileManager *fileManager, int ind, int fanout,
                      vector<pair<int, int>> data, int useBinarySearch);
    explicit BTreeSST(SSTFileManager *fileManager, string filename, int size, int useBinarySearch);
    bool get(const int& key, int &value);
    std::vector<std::pair<int, int>> scan(const int& key1, const int& key2);
};
#endif //KV_STORE_BTREESST_H
