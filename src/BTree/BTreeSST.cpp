#include "../../include/BTree/BTreeSST.h"
#include "string"
#include "../../include/constants.h"

BTreeSST::~BTreeSST() {
}

int BTreeSST::getSize() {
    return 0;
}

int btree_find(vector<vector<int>> btree, int needle, int fanout){
    int offset = 0;
    for(int layer = btree.size() - 1; layer > -1; layer--){
        //TODO: Delete layer 0
        for(int i =0; i < fanout; i ++){
            if(layer == 0 && i + offset >= btree[layer].size()){
                return -1;
            }else if(layer == 0 && btree[layer][i + offset] == needle){
                return i + offset;
            }else if(layer == 0 && btree[layer][i + offset] > needle){
                return offset + i;
            }
            if(i + offset >= btree[layer].size()){
                offset = btree[layer].size() * fanout;
                break;
            }
            if(needle <= btree[layer][i + offset]){
                offset = (i + offset) * fanout;
                break;
            }
        }
    }
}

void BTreeSST::constructBtree(vector<pair<int, int>> data){
    vector<int> keys = vector<int>();
    for(pair<int, int> p: data){
        keys.push_back(p.first);
    }
    vector<vector<int>> btree = vector<vector<int>>();
    auto current = keys;
    while(current.size() > fanout - 1){
        btree.push_back(current);
        auto level = vector<int>();
        for(int i = fanout - 1; i < current.size(); i += fanout){
            level.push_back(current[i]);
        }
        current = level;
    }
    //TODO: Delete layer 0 - No need to store in memory, pull from File
    btree.push_back(current);
    internal_btree = btree;
}

BTreeSST::BTreeSST(SSTFileManager *fileManager, int ind, int fanout, vector<pair<int, int>> data, int useBinarySearch) {
    this->fanout = fanout;
    this->constructBtree(data);
    internal_data = data;

    int sz = data.size();
    int *write_buf = new int[sz * 2];
    for (int i = 0; i < sz; i++)
    {
        write_buf[i * 2] = data[i].first;
        write_buf[i * 2 + 1] = data[i].second;
    }


    string fname = to_string(ind + 1) + ".sst";

    fileManager->write_file(write_buf, sz * ENTRY_SIZE, fname);

    delete[] write_buf;

}

BTreeSST::BTreeSST(SSTFileManager *fileManager, int ind, string filename, int useBinarySearch) {
    auto res = vector<pair<int, int>>();

    int total_read = PAGE_SIZE;

    int *data = new int[total_read/sizeof(int)];

    fileManager->scan(0, 1, filename, data);

    for (int ind = 0; ind < total_read/ENTRY_SIZE; ind++)
    {
        res.emplace_back(data[ind * 2], data[ind * 2 + 1]);
    }

    delete[] data;
    internal_data = res;
    this->fanout = 5;
    this->constructBtree(res);
}

bool BTreeSST::get(const int &key, int &value) {
    int pos = btree_find(internal_btree, key, fanout);
    if(pos == -1 || internal_data[pos].first > key) {
        return false;
    }
    value = internal_data[pos].second;
    return true;
}

std::vector<std::pair<int, int>> BTreeSST::scan(const int &key1, const int &key2) {
    auto res = vector<pair<int,int>>();
    int pos = btree_find(internal_btree, key1, fanout);
    if(pos == -1) {
        return res;
    }
    int cur = pos;
    while(internal_data[cur].first <= key2 and cur < internal_data.size()){
        res.emplace_back(internal_data[cur]);
        cur++;
    }
    return res;
}
