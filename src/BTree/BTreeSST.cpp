#include "../../include/BTree/BTreeSST.h"
#include "string"
#include "../../include/constants.h"
#include "math.h"

BTreeSST::~BTreeSST() {
}

int BTreeSST::getSize() {
    return this->size;
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

vector<pair<int, int>> BTreeSST::get_pages(int start_ind, int end_ind){
    auto res = vector<pair<int,int>>();
    if(start_ind > ceil((double) size/ (double) PAGE_SIZE) || start_ind > end_ind){
        return res;
    }

    int *data = new int[size * ENTRY_SIZE];

    fileManager->scan(start_ind, end_ind, filename, data);

    for (int ind = 0; ind < size; ind++)
    {
        if(data[ind * 2] != INT_MAX) {
            res.emplace_back(data[ind * 2], data[ind * 2 + 1]);
        }
    }
    delete[] data;
    return res;
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
    this->fileManager = fileManager;
    int sz = data.size();
    int *write_buf = new int[sz * 2];
    for (int i = 0; i < sz; i++)
    {
        write_buf[i * 2] = data[i].first;
        write_buf[i * 2 + 1] = data[i].second;
    }


    string fname = to_string(ind + 1) + ".sst";

    fileManager->write_file(write_buf, sz * ENTRY_SIZE, fname);
    size = data.size();
    filename = fname;
    delete[] write_buf;

}

BTreeSST::BTreeSST(SSTFileManager *fileManager, int fanout, string filename,int size, int useBinarySearch) {
    this->fileManager = fileManager;
    this->filename = filename;
    auto res = this->get_pages(0, size/PAGE_SIZE);
    size = res.size();
    this->fanout = fanout;
    this->constructBtree(res);
}


bool BTreeSST::get(const int &key, int &value) {
    if(size == 0){
        return false;
    }
    int pos = btree_find(internal_btree, key, fanout);
    int page = pos / PAGE_SIZE, offset = pos % PAGE_SIZE;
    auto page_data = this->get_pages(page, page);
    if(pos == -1 || page_data[offset].first > key) {
        return false;
    }
    value = page_data[offset].second;
    return true;
}

std::vector<std::pair<int, int>> BTreeSST::scan(const int &key1, const int &key2) {
    auto res = vector<pair<int,int>>();
    int pos = btree_find(internal_btree, key1, fanout);
    if(pos == -1) {
        return res;
    }
    int cur = pos;
    int page = pos / PAGE_SIZE, offset = pos % PAGE_SIZE;
    auto page_data = this->get_pages(page, page);
    while(page_data[offset].first <= key2 and cur < this->getSize()){
        res.emplace_back(page_data[offset]);
        cur++;
        offset = cur % PAGE_SIZE;
        if(offset == 0){
            page++;
            page_data = this->get_pages(page, page);
        }
    }
    return res;
}
