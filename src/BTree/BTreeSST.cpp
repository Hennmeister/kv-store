#include "../../include/BTree/BTreeSST.h"
#include "string"
#include <cstring>
#include "../../include/constants.h"
#include "math.h"
#include "../../include/util.h"
#include <climits>
#include <stdio.h>

int negatives = 0;
int total = 0;
int false_positive =0;

using namespace std;

BTreeSST::~BTreeSST(){
//    fileManager->delete_file(this->filename);
//    cout << "total: "<< total<< " negatives: " << negatives << " false positives: " << false_positive << endl;
};

void BTreeSST::delete_sst(){
    fileManager->delete_file(this->filename);
}

int BTreeSST::getSize() const {
    return this->size;
}

// Find the position of an element which lower bounds the given needle in the btree
int btree_find(const vector<vector<int>>& btree, int needle, int fanout){
    int offset = 0;
    for(int layer = (int) btree.size() - 1; layer > -1; layer--){
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
        auto cur_page = this->get_page_raw(mid);
        int ind = binary_search_raw(cur_page.first, cur_page.second, target, discard);

        if (ind != -1 && cur_page.first[ind * 2] == target)
        {
            delete[] cur_page.first;
            return mid * PAGE_NUM_ENTRIES + ind;
        }
        else if (cur_page.first[(cur_page.second - 1) * 2] < target)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
        delete[] cur_page.first;
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
        auto cur_page = this->get_page(mid);
        auto elem = lower_bound(cur_page.begin(), cur_page.end(), target,
                                [](const pair<int, int> &info, double value)
                                {
                                    return info.first < value;
                                });
        int ind = min((int) distance(cur_page.begin(), elem), (int) cur_page.size() - 1);

        bool is_lb = true;
        if(mid != 0 && ind == 0){
            auto prev_page = this->get_page(mid -1);
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
// NOTE: This DOES NOT store data in the buffer by virtue of calling filemanager->scan
vector<pair<int, int>> BTreeSST::get_pages(int start_ind, int end_ind){
    auto res = vector<pair<int,int>>();
    if(start_ind >= ceil((double) size/ (double) PAGE_NUM_ENTRIES) || start_ind > end_ind){
        return res;
    }

    int num_entries = (end_ind - start_ind + 1) * PAGE_NUM_ENTRIES;
    res = vector<pair<int,int>>(num_entries);
    int *data = new int[num_entries * PAGE_NUM_ENTRIES * 2];

    // Ignore internal node pages
    fileManager->scan(this->internal_node_pages + start_ind,
                      this->internal_node_pages+ end_ind, filename, data);
    int count = 0;
    for (int ind = 0; ind < num_entries; ind++)
    {
        if(data[ind * 2] != INT_MAX) {
            count++;
            res[ind] = pair(data[ind * 2], data[ind * 2 + 1]);
        }else{
            break;
        }
    }
    res.resize(count);
    delete[] data;
    return res;
}

// Get a single entry_data from the file manager and parse it into a vector of key-value pairs.
// NOTE: This DOES store data in the buffer by virtue of calling filemanager->get_page
vector<pair<int,int>> BTreeSST::get_page(int page_ind){
    auto res = vector<pair<int,int>>();
    if(page_ind >= ceil((double) size/ (double) PAGE_NUM_ENTRIES)){
        return res;
    }
    int num_entries = PAGE_NUM_ENTRIES;
    if((page_ind + 1) * PAGE_NUM_ENTRIES > this->size){
        num_entries = this->size % PAGE_NUM_ENTRIES;
    }
    res = vector<pair<int,int>>(num_entries);

    int *data = new int[PAGE_NUM_ENTRIES * 2];

    // Ignore internal node pages
    fileManager->get_page(this->internal_node_pages + page_ind, filename, data);

    for (int ind = 0; ind < num_entries; ind++)
    {
        if(data[ind * 2] != INT_MAX) {
            res[ind] = pair(data[ind * 2], data[ind * 2 + 1]);
        }
    }
    delete[] data;
    return res;
}


pair<int*, int> BTreeSST::get_page_raw(int page_ind){
    if(page_ind >= ceil((double) size/ (double) PAGE_NUM_ENTRIES)){
        return pair(new int[0], 0);
    }

    int num_entries = PAGE_NUM_ENTRIES;
    if(page_ind * PAGE_NUM_ENTRIES > this->size){
        num_entries = this->size % PAGE_NUM_ENTRIES;
    }

    int *data = new int[num_entries * 2];

    // Ignore internal node pages
    fileManager->get_page(this->internal_node_pages + page_ind, filename, data);

    return pair(data, num_entries);
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

// New SST creation - construct btree and write data to file. (Should only be called with small SSTs)
BTreeSST::BTreeSST(SSTFileManager *fileManager, int ind, int fanout, vector<pair<int, int>> data,
                   int useBinarySearch, int filter_bits_per_entry) {
    int write_counter = 0;
    this->fanout = fanout;
    this->constructBtree(data);
    this->fileManager = fileManager;
    this->size = data.size();
    int data_pages = ceil((double) this->size/ (double) PAGE_NUM_ENTRIES);

    // Calculate how to write out internal nodes and required padding
    this->internal_node_pages = ceil((double) (get_internal_node_count() + this->internal_btree.size() + 1) * sizeof(int)
            / (double) PAGE_SIZE);

    int internal_node_ints = internal_node_pages * (PAGE_SIZE / sizeof(int));

    // construct bloom filter and init with initial data
    BloomFilter *filter = new BloomFilter(size, filter_bits_per_entry);
    auto serial_info = filter->serialize();
    delete[] serial_info.first;
    int bloom_filter_num_pages = serial_info.second;
    int write_total_ints = internal_node_ints + (data_pages * PAGE_NUM_ENTRIES) * 2 +
                           bloom_filter_num_pages * (PAGE_SIZE/sizeof(int));
    // num_seeds + seeds + num bits + bits
    int *write_buf = new int[write_total_ints];
    memset(write_buf, 0, write_total_ints * sizeof(int));

    write_buf[0] += this->internal_btree.size();
    write_counter++;
    int counter = 1;
    for( auto level: this->internal_btree){
        for(int val : level){
            write_buf[counter] = val;
            write_counter++;
            counter ++;
        }
        write_buf[counter] = INT_MAX - 1;
        write_counter++;
        counter ++;
    }

    // Pad remaining internal_node pages with data
    while(counter % (PAGE_SIZE/sizeof(int)) != 0){
        write_buf[counter] = INT_MAX - 1;
        counter ++;
        write_counter++;
    }

    for (int i = 0; i < data_pages * PAGE_NUM_ENTRIES; i++)
    {
        if(i >= data.size()){
            write_buf[counter + i * 2] = INT_MAX;
            write_buf[counter + i * 2 + 1] = 0;
        }else {
            write_buf[counter + i * 2] = data[i].first;
            write_buf[counter + i * 2 + 1] = data[i].second;
            filter->insert(data[i].first);
        }
        write_counter++;
        write_counter++;
    }

    // write out bloom filter
    pair<int *, int> serialized_filter_pair = filter->serialize();
    int *serialized_filter = serialized_filter_pair.first;
    int num_filter_pages = serialized_filter_pair.second;
    this->num_filter_pages = num_filter_pages;
    this->filter_start_page =  ((internal_node_ints + (data_pages * PAGE_NUM_ENTRIES) * 2) * sizeof(int)) / PAGE_SIZE;
    memcpy(&write_buf[filter_start_page * PAGE_SIZE / sizeof(int)], serialized_filter,
           num_filter_pages * PAGE_SIZE);
    write_counter += (num_filter_pages * PAGE_SIZE)/sizeof(int);

    string fname = to_string(ind + 1) + ".sst";
    int *meta = new int[PAGE_SIZE/sizeof(int)];
    memset(meta, 0, PAGE_SIZE);
    meta[0] = fanout;
    meta[1] = this->internal_node_pages;
    meta[2] = data.size();
    // bloom filter start page
    meta[3] = filter_start_page;
    // size of bloom filter data in pages
    meta[4] = num_filter_pages;

    // note that we multiply by 2 since each entry has two ints
    if(write_counter != ((internal_node_ints + (data_pages * PAGE_NUM_ENTRIES) * 2) * sizeof(int) +
                         num_filter_pages * PAGE_SIZE)/sizeof(int)){
        fprintf(stderr, "ERROR\n");
    }
    fileManager->write_file(write_buf,
                            write_total_ints * sizeof(int),
                            fname, meta);
    this->useBinary = useBinarySearch;
    this->filename = fname;
    delete[] write_buf;
    delete[] meta;
    delete[] serialized_filter;
    delete filter;
}


int BTreeSST::get_internal_node_count(){
    int total_internal_nodes = 0;
    for(const vector<int>& level: this->internal_btree){
        total_internal_nodes += level.size();
    }
    return total_internal_nodes;
}

vector<vector<int>> load_btree(int* data, int internal_node_pages){
    vector<vector<int>> tree = vector<vector<int>>();
    vector<int> tmp_level = vector<int>();
    for(int read_counter = 1; read_counter < internal_node_pages * (PAGE_SIZE / sizeof(int)); read_counter++){
        if(data[read_counter] == INT_MAX - 1 && tmp_level.size() == 0){
            break;
        }
        else if(data[read_counter] == INT_MAX - 1){
            tree.push_back(tmp_level);
            tmp_level = vector<int>();
        }else{
            tmp_level.emplace_back(data[read_counter]);
        }
    }
    return tree;
}


// Load existing sst, read internal nodes from file and load into memory
BTreeSST::BTreeSST(SSTFileManager *fileManager, string filename, int size, int useBinarySearch) {
    this->fileManager = fileManager;
    this->filename = filename;
    this->useBinary = useBinarySearch;
    int *meta = new int[PAGE_SIZE/sizeof(int)];
    this->fileManager->get_metadata(meta, filename);
    this->fanout = meta[0];
    this->internal_node_pages = meta[1];
    this->filter_start_page = meta[3];
    this->num_filter_pages = meta[4];

    int *data = new int[this->internal_node_pages * (PAGE_SIZE / sizeof(int))];

    // Ignore internal node pages
    fileManager->scan(0,
                      this->internal_node_pages - 1, filename, data);

    // Load internal nodes
    this->internal_btree = load_btree(data, this->internal_node_pages);

    // Remove metadata from file size (this->size is number of entries)
    this->size = meta[2];
    delete[] data;
    delete[] meta;
}


bool BTreeSST::get(const int &key, int &value) {
    if(size == 0){
        return false;
    }
    BloomFilter *filter = get_bloom_filter();
    total++;
    if (!filter->testMembership(key)) {
        negatives++;
        return false;
    }
    delete filter;

    if(useBinary){
        int cur = this->binary_scan(key);
        if(cur == -1){
            false_positive++;
            return false;
        }
        int page = cur / PAGE_NUM_ENTRIES;
        auto page_data = this->get_page_raw(page);
        if(page_data.first[(cur % PAGE_NUM_ENTRIES) * 2] == key){
            value = page_data.first[(cur % PAGE_NUM_ENTRIES) * 2 + 1];
            delete[] page_data.first;
            return true;
        }
        false_positive++;
        return false;
    }else {
        int pos = btree_find(internal_btree, key, fanout);
        int page = pos / PAGE_NUM_ENTRIES, offset;
        auto page_data = this->get_page_raw(page);
        int cur = pos;

        // Check all elements in node pointed to by btree (i.e. lowest level of a virtually clustered btree)
        for (int i = 0; i <= fanout; i++) {
            offset = cur % PAGE_NUM_ENTRIES;
            if (offset == 0 && i != 0) {
                if (cur >= this->getSize()) {
                    false_positive++;
                    delete[] page_data.first;
                    return false;
                }
                page++;
                page_data = this->get_page_raw(page);
            }
            if (page_data.first[offset * 2] == key) {
                value = page_data.first[offset * 2 + 1];
                delete[] page_data.first;
                return true;
            }
            cur++;

        }
    }
    false_positive++;
    return false;
}

// Get pages used to avoid storing data in buffer pool
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

BloomFilter *BTreeSST::get_bloom_filter() {
    int *data_buf = new int[num_filter_pages * PAGE_SIZE];
    fileManager->scan(filter_start_page, (filter_start_page + num_filter_pages) - 1,
                      filename, data_buf, true);
    BloomFilter *fltr = new BloomFilter(data_buf);
    delete[] data_buf;
    return fltr;
}
