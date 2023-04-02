#include "../../include/BTree/BTreeSST.h"
#include "string"
#include "../../include/constants.h"
#include "math.h"
#include "../../include/util.h"

BTreeSST::~BTreeSST() = default;

int BTreeSST::getSize() const {
    return this->size;
}

// Find the position of an element which lower bounds the given needle in the btree
int btree_find(vector<vector<int>> btree, int needle, int fanout){
    int offset = 0;
    for(int layer = btree.size() - 1; layer > -1; layer--){
        int i;
        for(i =0; i < fanout; i ++){
            if(i + offset >= btree[layer].size()){
                break;
            }
            if(needle <= btree[layer][i + offset]){
                break;
            }
        }
        offset = (i + offset) * fanout;
    }
    return offset;
}

// Use binary search to find the position of an element, -1 if it does not exist
int BTreeSST::binary_scan(int target){
    int n_pages = ceil((double) this->getSize() / (double) PAGE_NUM_ENTRIES);
    int left = 0;
    int right = n_pages - 1;
    int discard;
    while (left <= right)
    {
        int mid = left + (right - left) / 2;
        auto cur_page = this->get_pages(mid, mid);
        int ind = binary_search(cur_page, target, discard);
        if (cur_page[ind].first == target)
        {
            return mid * PAGE_NUM_ENTRIES + ind;
        }
        else if (cur_page[cur_page.size() - 1].first < target)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }
    return -1;

}

int BTreeSST::binary_lower_bound(int target){
    int n_pages = ceil((double) this->getSize() / (double) PAGE_NUM_ENTRIES);
    int left = 0;
    int right = n_pages - 1;
    while (left <= right)
    {
        int mid = left + (right - left) / 2;
        auto cur_page = this->get_pages(mid, mid);
        auto elem = lower_bound(cur_page.begin(), cur_page.end(), target,
                                [](const pair<int, int> &info, double value)
                                {
                                    return info.first < value;
                                });
        int ind = distance(cur_page.begin(), elem);

        bool is_lb = true;
        if(mid != 0 && ind == 0){
            auto prev_page = this->get_pages(mid -1, mid -1);
            if(prev_page[prev_page.size() - 1].first >= target){
                is_lb = false;
            }
        }

        if(!is_lb){
            right = mid - 1;
        }
        else if (cur_page[ind].first >= target)
        {
            return mid * PAGE_NUM_ENTRIES + ind;
        }
        else if (cur_page[cur_page.size() - 1].first < target)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }
    return -1;

}

// Get a set of pages from the file manager and parse it into a vector of key-value pairs.
vector<pair<int, int>> BTreeSST::get_pages(int start_ind, int end_ind){
    auto res = vector<pair<int,int>>();
    if(start_ind >= ceil((double) size/ (double) PAGE_NUM_ENTRIES) || start_ind > end_ind){
        return res;
    }
    int num_entries = (end_ind - start_ind + 1) * PAGE_NUM_ENTRIES;
    int *data = new int[num_entries * PAGE_NUM_ENTRIES * 2];

    fileManager->scan(start_ind, end_ind, filename, data);

    for (int ind = 0; ind < num_entries; ind++)
    {
        if(data[ind * 2] != INT_MAX) {
            res.emplace_back(data[ind * 2], data[ind * 2 + 1]);
        }
    }
    delete[] data;
    return res;
}

// Construct a btree from the given key-value pair data (assuming sorted order)
void BTreeSST::constructBtree(const vector<pair<int, int>>& data){
    vector<int> keys = vector<int>();
    // This acts as the lowest level of the btree - does not get added to the final btree.
    for(pair<int, int> p: data){
        keys.push_back(p.first);
    }

    vector<vector<int>> btree = vector<vector<int>>();
    auto current = keys;

    // Loop until just 1 root node less than fanout
    while(current.size() > fanout - 1){
        if(current != keys) {
            btree.push_back(current);
        }
        auto level = vector<int>();
        for(int i = fanout - 1; i < current.size(); i += fanout){
            level.push_back(current[i]);
        }
        current = level;
    }
    if(!current.empty() && current != keys) {
        btree.push_back(current);
    }
    this->internal_btree = btree;
}

// New SST creation - construct btree and write data to file.
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
    int *meta = new int[PAGE_SIZE/sizeof(int)];
    meta[0] = fanout;
    fileManager->write_file(write_buf, sz * ENTRY_SIZE, fname, meta);
    this->useBinary = useBinarySearch;
    this->size = data.size();
    this->filename = fname;
    delete[] write_buf;
    delete[] meta;

}

// Load existing sst, read data from file and construct btree in memory.
BTreeSST::BTreeSST(SSTFileManager *fileManager, string filename,int size, int useBinarySearch) {
    this->fileManager = fileManager;
    this->filename = filename;
    this->useBinary = useBinarySearch;
    int *meta = new int[PAGE_SIZE/sizeof(int)];
    this->fileManager->get_metadata(meta, filename);
    this->fanout = meta[0];
    // Remove metadata from file size (this->size is number of entries)
    this->size = (size - PAGE_SIZE)/ENTRY_SIZE;
    auto res = this->get_pages(0, ceil((double) this->size/ (double) PAGE_NUM_ENTRIES) - 1);
    // Required in case some data was padded (i.e. for memtable drop)
    this->size = res.size();
    this->constructBtree(res);
    delete[] meta;
}


bool BTreeSST::get(const int &key, int &value) {
    if(size == 0){
        return false;
    }
    if(useBinary){
        int cur = this->binary_scan(key);
        if(cur == -1){
            return false;
        }
        int page = cur / PAGE_NUM_ENTRIES;
        auto page_data = this->get_pages(page, page);
        if(page_data[cur % PAGE_NUM_ENTRIES].first == key){
            value = page_data[cur % PAGE_NUM_ENTRIES].second;
            return true;
        }
        return false;
    }else {
        int pos = btree_find(internal_btree, key, fanout);
        int page = pos / PAGE_NUM_ENTRIES, offset;
        auto page_data = this->get_pages(page, page);

        int cur = pos;

        // Check all elements in node pointed to by btree (i.e. lowest level of a virtually clustered btree)
        for (int i = 0; i <= fanout; i++) {
            offset = cur % PAGE_NUM_ENTRIES;
            if (offset == 0 && i != 0) {
                if (cur >= this->getSize()) {
                    return false;
                }
                page++;
                page_data = this->get_pages(page, page);
            }
            if (page_data[offset].first == key) {
                value = page_data[offset].second;
                return true;
            }
            cur++;

        }
    }
    return false;
}

std::vector<std::pair<int, int>> BTreeSST::scan(const int &key1, const int &key2) {
    auto res = vector<pair<int,int>>();
    int cur, page, offset;

    if(useBinary){
        int pos = this->binary_lower_bound(key1);
        if(pos == -1){
            return res;
        }
        cur = pos;
        page = cur / PAGE_NUM_ENTRIES, offset = cur % PAGE_NUM_ENTRIES;
    }else{
        int pos = btree_find(internal_btree, key1, fanout);
        if(pos == -1) {
            return res;
        }
        cur = pos;
        page = cur / PAGE_NUM_ENTRIES, offset = cur % PAGE_NUM_ENTRIES;
        auto page_data = this->get_pages(page, page);

        // First find elem which is at least key1
        for(int i = 0; i <= fanout; i++){
            offset = cur % PAGE_NUM_ENTRIES;
            if(offset == 0 && i != 0){
                page++;
                page_data = this->get_pages(page, page);
            }
            if(cur >= this->getSize() || page_data[offset].first >= key1) {
                break;
            }
            cur++;
        }
    }


    auto page_data = this->get_pages(page, page);
    while(cur < this->getSize() && page_data[offset].first <= key2){
        res.emplace_back(page_data[offset]);
        cur++;
        offset = cur % PAGE_NUM_ENTRIES;
        if(offset == 0){
            page++;
            page_data = this->get_pages(page, page);
        }
    }
    return res;
}
