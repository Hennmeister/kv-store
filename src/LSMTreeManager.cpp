#include "../include/LSMTreeManager.h"
#include "string"
#include "../include/util.h"
#include "../include/constants.h"
#include <math.h>
#include <cstring>
#include <climits>

bool LSMTreeManager::get(const int &key, int &value) {
    for (int i = 0; i < levels.size(); i ++) {
        auto level = levels[i];
        {
            for (auto sst: level) {
                if (sst->get(key, value)) {
                    return true;
                }
            }
        }
    }
    return false;
}


LSMTreeManager::LSMTreeManager(SSTFileManager *fileManager, int newFanout, int useBinarySearch, int memtable_size,
                               int filter_bits_per_entry) {
    auto files = fileManager->get_files();
    // Reverse SST order by filename so that newer SSTs are first.
    std::sort(files.begin(), files.end(), sortByFname);
    std::reverse(files.begin(), files.end());

    this->useBinary = useBinarySearch;

    this->fileManager = fileManager;
    this->sst_counter = 0;
    this->newFanout = newFanout;
    this->filter_bits_per_entry = filter_bits_per_entry;
    this->memtable_size = memtable_size;
    this->levels = vector<vector<BTreeSST*>>();
    this->levels.push_back(vector<BTreeSST*>());

    auto tmp_ssts = vector<BTreeSST *>();
    int i = 0;
    for(const pair<string, int>& fileDat : files){
        tmp_ssts.push_back(new BTreeSST(fileManager, fileDat.first, fileDat.second,
                                        useBinary));
        total_entries += tmp_ssts[i]->getSize();
        int file_num = stoi(fileDat.first.substr(0, fileDat.first.find('.')));
        if(file_num - 1 >= sst_counter){
            sst_counter = file_num;
        }
        i++;
    }

    // Construct levels based on file sizes
    int start_log = log2(memtable_size/ENTRY_SIZE);
    for(BTreeSST* sst: tmp_ssts){
        int level = max( (int) (ceil(log2((double) sst->getSize())) - start_log), 0);
        while(this->levels.size() <= level){
            this->levels.push_back(vector<BTreeSST*>());
        }
        this->levels[level].push_back(sst);
    }
}

std::vector<std::pair<int, int>> LSMTreeManager::scan(const int &key1, const int &key2) {
    auto res = vector<pair<int, int>>();

    for (auto level: levels)
    {
        for(auto sst: level) {
            res = priority_merge(res, sst->scan(key1, key2));
        }
    }
    return res;
}

bool LSMTreeManager::add_sst(std::vector<std::pair<int, int>> data) {
    auto* new_sst = new BTreeSST(fileManager, sst_counter, newFanout, data, useBinary, filter_bits_per_entry);
    sst_counter++;
    levels[0].push_back(new_sst);
    total_entries += new_sst->getSize();
    if(levels[0].size() == 2){
        return this->compact_tree(0);
    }
    return true;
}

LSMTreeManager::~LSMTreeManager() {
//     Free memory
    for (auto level: levels)
    {
        for(auto sst: level) {
            delete sst;
        }
    }
}

// Helper function for combine SST
void flush_data(vector<pair<int,int>> &res, int& btree_ctr, int newFanout, SSTFileManager *fileManager,
                vector<int> &lowest_level, int& total_size, int& write_pg_ctr, string fname){
    int *write_data_buf = new int[PAGE_SIZE/sizeof(int)];
    for(int i = 0; i < PAGE_NUM_ENTRIES; i++){
        if((btree_ctr + 1) % newFanout == 0){
            lowest_level.push_back(res[i].first);
        }
        btree_ctr++;
        write_data_buf[i * 2] = res[i].first;
        write_data_buf[i * 2 + 1] = res[i].second;
    }
    total_size += PAGE_NUM_ENTRIES;
    res.erase(res.begin(), res.begin() + PAGE_NUM_ENTRIES);
    fileManager->write_page(write_data_buf, PAGE_SIZE, write_pg_ctr, fname);
    write_pg_ctr++;
    delete[] write_data_buf;
}


// Apologies for the long function - it requires many buffers to be persisted and the tradeoff of readability was made
// In order to enable performance. Need 2 maintain 2 buffers throughout the function
BTreeSST* LSMTreeManager::combine_SST(BTreeSST* newer, BTreeSST* older){
    int newer_size = newer->getSize();
    int older_size = older->getSize();

    // create new bloom filter, slightly overestimating size due to key collisions
    BloomFilter *filter = new BloomFilter(newer_size + older_size ,this->filter_bits_per_entry);

    // Calculate constants
    int internal_node_ints_newer = newer->get_internal_node_count() + newer->internal_btree.size();
    int internal_node_ints_older = older->get_internal_node_count() + older->internal_btree.size();

    int total_internal_ints_max = internal_node_ints_newer + internal_node_ints_older;
    int internal_node_pages = ceil((double) total_internal_ints_max* sizeof(int) / (double) PAGE_SIZE);
    int internal_node_ints_padded = internal_node_pages * (PAGE_SIZE / sizeof(int));

    int newer_pages = ceil((double) newer_size / (double) PAGE_NUM_ENTRIES);
    int older_pages = ceil((double) older_size / (double) PAGE_NUM_ENTRIES);

    int newer_pg_ctr = 0;
    int older_pg_ctr = 0;
    int write_pg_ctr = internal_node_pages + 1;
    int total_size = 0;

    int btree_ctr = 0;
    vector<int> lowest_level = vector<int>();
    int *write_data_buf = new int[PAGE_SIZE/sizeof(int)];
    int *internal_nodes_buf = new int[internal_node_ints_padded];
    int *meta = new int[PAGE_SIZE/sizeof(int)];
    memset(meta, 0, PAGE_SIZE);
    meta[0] = newFanout;
    meta[1] = internal_node_pages;
    string fname = to_string(sst_counter + 1) + ".sst";
    memset(internal_nodes_buf, 0, internal_node_pages * PAGE_NUM_ENTRIES);
    
    // Write out initial file
    fileManager->write_file(internal_nodes_buf,
                            (internal_node_pages) * PAGE_NUM_ENTRIES,
                            fname, meta);

    vector<pair<int,int>> res = vector<pair<int, int>>();

    // Use get pages to avoid storing data into the buffer pool
    auto master = newer->get_pages(newer_pg_ctr, newer_pg_ctr);
    auto older_page = older->get_pages(older_pg_ctr, older_pg_ctr);

    int ind0 = 0;
    int ind1 = 0;

    // Construct resulting pages with 2 page buffers ----------------------------------
    while(newer_pg_ctr < newer_pages && older_pg_ctr < older_pages){
        // Exactly the same as priority merge
        while (ind0 < master.size() && ind1 < older_page.size())
        {
            if (master[ind0].first > older_page[ind1].first)
            {
                res.emplace_back(older_page[ind1]);
                filter->insert(older_page[ind1].first);
                ind1++;
            }
            else if (master[ind0].first < older_page[ind1].first)
            {
                res.emplace_back(master[ind0]);
                filter->insert(master[ind0].first);
                ind0++;
            }
            else
            {
                if(master[ind0].second != INT_MIN) {
                    res.emplace_back(master[ind0]);
                    filter->insert(master[ind0].first);
                }
                ind0++;
                ind1++;
            }
        }
        if (ind1 == older_page.size())
        {
           older_pg_ctr++;
           ind1 = 0;
           older_page = older->get_pages(older_pg_ctr, older_pg_ctr);
        }
        if (ind0 == master.size())
        {
            newer_pg_ctr++;
            ind0 = 0;
            master = newer->get_pages(newer_pg_ctr, newer_pg_ctr);
        }

        // Flush data
        while(res.size() >= PAGE_NUM_ENTRIES){
            flush_data(res, btree_ctr, newFanout, fileManager, lowest_level,
                       total_size, write_pg_ctr, fname);
        }
    }

    // If either SST is not exhausted
    while(newer_pg_ctr != newer_pages){
        for(int i = ind0; i < master.size(); i ++){
            res.emplace_back(master[i]);
            filter->insert(master[i].first);
        }
        ind0 = 0;
        newer_pg_ctr++;
        if(newer_pg_ctr <= newer_pages){
            master = newer->get_pages(newer_pg_ctr, newer_pg_ctr);
        }
        // Flush entry_data
        while(res.size() >= PAGE_NUM_ENTRIES){
            flush_data(res, btree_ctr, newFanout, fileManager, lowest_level,
                       total_size, write_pg_ctr, fname);
        }
    }
    while(older_pg_ctr != older_pages){
        for(int i = ind1; i < older_page.size(); i ++){
            res.emplace_back(older_page[i]);
            filter->insert(older_page[i].first);
        }
        ind1 = 0;
        older_pg_ctr++;
        if(older_pg_ctr <= older_pages){
            older_page = older->get_pages(older_pg_ctr, older_pg_ctr);
        }
        // Flush page
        while(res.size() >= PAGE_NUM_ENTRIES){
            flush_data(res, btree_ctr, newFanout, fileManager, lowest_level,
                       total_size, write_pg_ctr, fname);
        }
    }

    // Flush any extra data
    total_size += res.size();
    for(pair<int, int> entry: res){
        if((btree_ctr + 1) % newFanout == 0){
            lowest_level.push_back(entry.first);
        }
        btree_ctr++;
    }
    while(res.size() > 0) {
        for (int i = 0; i < PAGE_NUM_ENTRIES; i++) {
            if (i >= res.size()) {
                write_data_buf[i * 2] = INT_MAX;
                write_data_buf[i * 2 + 1] = 0;
            } else {
                write_data_buf[i * 2] = res[i].first;
                write_data_buf[i * 2 + 1] = res[i].second;
            }
        }
        res.erase(res.begin(), res.begin() + min((int) res.size(), PAGE_NUM_ENTRIES));
        fileManager->write_page(write_data_buf, PAGE_SIZE, write_pg_ctr, fname);
        write_pg_ctr++;
    }
    delete[] write_data_buf;

    // Construct BTREE ----------------------------------
    vector<vector<int>> btree = vector<vector<int>>();
    auto current = lowest_level;

    // Loop until just 1 root node less than fanout
    while(current.size() > newFanout - 1){
        btree.push_back(current);
        auto level = vector<int>();
        for(int i = newFanout - 1; i < current.size(); i += newFanout){
            level.push_back(current[i]);
        }
        current = level;
    }
    if(!current.empty()) {
        btree.push_back(current);
    }

    // Write out BTREE ----------------------------------
    internal_nodes_buf[0] += btree.size();
    int counter = 1;
    for( auto level: btree){
        for(int val : level){
            internal_nodes_buf[counter] = val;
            counter ++;
        }
        internal_nodes_buf[counter] = INT_MAX - 1;
        counter ++;
    }

    while(counter % (PAGE_SIZE/sizeof(int)) != 0){
        internal_nodes_buf[counter] = INT_MAX - 1;
        counter ++;
    }

    fileManager->write_page(internal_nodes_buf, PAGE_SIZE * internal_node_pages, 1, fname);

    // write out bloom filter
    pair<int *, int> serialized_filter_pair = filter->serialize();
    int *serialized_filter = serialized_filter_pair.first;
    int num_filter_pages = serialized_filter_pair.second;
    fileManager->write_page(serialized_filter, PAGE_SIZE * num_filter_pages,
                            write_pg_ctr, fname);

    // Update metadata with new size
    meta[2] = total_size;
    // bloom filter start
    meta[3] = write_pg_ctr - 1;
    // bloom filter size
    meta[4] = num_filter_pages;
    fileManager->write_page(meta, PAGE_SIZE, 0, fname);
    delete filter;
    delete[] serialized_filter;
    delete[] internal_nodes_buf;
    delete[] meta;
    return new BTreeSST(fileManager, fname, total_size, useBinary);
}

// In memory merge --------------------------------------------------
//BTreeSST* LSMTreeManager::combine_SST(BTreeSST* newer, BTreeSST* older){
//    auto data_newer = newer->scan(INT_MIN, INT_MAX);
//    auto data_older = older->scan(INT_MIN, INT_MAX);
//
//    auto result_data = priority_merge(data_newer, data_older);
//    return new BTreeSST(fileManager, sst_counter, newFanout, result_data, useBinary);
//}

bool LSMTreeManager::compact_tree(int level) {
    auto res = combine_SST(levels[level][1], levels[level][0]);
    sst_counter++;

    // Delete ssts and free memory
    levels[level][1]->delete_sst();
    levels[level][0]->delete_sst();
    delete levels[level][1];
    delete levels[level][0];
    levels[level].clear();

    // Create a new level if it exists
    if(levels.size() == level + 1){
        auto new_level = vector<BTreeSST*>();
        new_level.push_back(res);
        levels.push_back(new_level);
    }else{
        levels[level+1].push_back(res);
        if(levels[level+1].size() == 2){
            return compact_tree(level + 1);
        }
    }
    return true;
}
