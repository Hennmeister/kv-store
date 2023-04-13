#ifndef KV_STORE_BTREESST_H
#define KV_STORE_BTREESST_H
#include "../Base/SSTFileManager.h"
#include "string"
#include "../../src/BloomFilter/BloomFilter.h"

using namespace std;
class BTreeSST{
private:
    int fanout;
    int size;
    string filename;
    SSTFileManager *fileManager;
    int useBinary;
    int internal_node_pages;
    int filter_start_page;
    int num_filter_pages;
    void constructBtree(const vector<pair<int, int>>& data);
    int binary_scan(int key);
    int binary_lower_bound(int key);
    BloomFilter get_bloom_filter();
public:
    vector<vector<int>> internal_btree;
    ~BTreeSST();
    int getSize() const;
    void delete_sst();
    int get_internal_node_count();
    vector<pair<int, int>> get_pages(int start_ind, int end_ind);
    vector<pair<int, int>> get_page(int page_ind);
    explicit BTreeSST(SSTFileManager *fileManager, int ind, int fanout,
                      vector<pair<int, int>> data, int useBinarySearch, int filter_bits_per_entry);
    explicit BTreeSST(SSTFileManager *fileManager, string filename, int size, int useBinarySearch);
    bool get(const int& key, int &value);
    std::vector<std::pair<int, int>> scan(const int& key1, const int& key2);
};
#endif //KV_STORE_BTREESST_H
