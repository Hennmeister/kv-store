#include "../include/LSMTreeManager.h"
#include "string"
#include "../include/util.h"
#include "../include/constants.h"
#include <math.h>
#include <climits>

bool LSMTreeManager::get(const int &key, int &value) {
    for (auto level: levels)
    {
        for(auto sst: level) {
            if (sst->get(key, value)) {
                return true;
            }
        }
    }
    return false;
}

bool sortByFname(const pair<string,int> &a,
                 const pair<string,int> &b)
{
    int pos_a = stoi(a.first.substr(0, a.first.size()-4));
    int pos_b = stoi(b.first.substr(0, b.first.size()-4));
    return (pos_a < pos_b);
}

LSMTreeManager::LSMTreeManager(SSTFileManager *fileManager, int newFanout, int useBinarySearch, int memtable_size) {
    auto files = fileManager->get_files();
    // Reverse SST order by filename so that newer SSTs are first.
    std::sort(files.begin(), files.end(), sortByFname);
    std::reverse(files.begin(), files.end());

    this->useBinary = useBinarySearch;

    this->fileManager = fileManager;
    this->sst_counter = 0;
    this->newFanout = newFanout;
    this->levels = vector<vector<BTreeSST*>>();
    this->levels.push_back(vector<BTreeSST*>());

    auto tmp_ssts = vector<BTreeSST *>();
    int i = 0;
    for(const pair<string, int>& fileDat : files){
        tmp_ssts.push_back(new BTreeSST(fileManager, fileDat.first, fileDat.second, useBinary));
        total_entries += tmp_ssts[i]->getSize();
        int file_num = stoi(fileDat.first.substr(0, fileDat.first.find('.')));
        if(file_num -1 > sst_counter){
            sst_counter = file_num - 1;
        }
        i++;
    }


    int start_log = log2(memtable_size);
    for(BTreeSST* sst: tmp_ssts){
        int level = ceil(log2((double) sst->getSize())) - start_log;
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
//    if (data.size() % PAGE_NUM_ENTRIES != 0)
//        return false;
    auto* new_sst = new BTreeSST(fileManager, sst_counter, newFanout, data, useBinary);
    sst_counter++;
    levels[0].push_back(new_sst);
    total_entries += new_sst->getSize();
    if(levels[0].size() == 2){
        return this->compact_tree(0);
    }
    return true;
}

LSMTreeManager::~LSMTreeManager() {
    // Free memory
//    for (BTreeSST* sst: ssts)
//    {
//        delete sst;
//    }
}


// Apologies for the long function - it requires many buffers to be persisted and the tradeoff of readability was made
// In order to enable performance.
BTreeSST* LSMTreeManager::combine_SST(BTreeSST* newer, BTreeSST* older){
    int newer_size = newer->getSize();
    int older_size = older->getSize();

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
    meta[0] = newFanout;
    meta[1] = internal_node_pages;
    string fname = to_string(sst_counter + 1) + ".sst";

    // Write out initial file
    fileManager->write_file(internal_nodes_buf,
                            (internal_node_pages) * PAGE_NUM_ENTRIES,
                            fname, meta);

    vector<pair<int,int>> res = vector<pair<int, int>>();;
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
                ind1++;
            }
            else if (master[ind0].first < older_page[ind1].first)
            {
                res.emplace_back(master[ind0]);
                ind0++;
            }
            else
            {
                if(master[ind0].second != INT_MIN) {
                    res.emplace_back(master[ind0]);
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
        }
    }

    // If either SST is not exhausted
    while(newer_pg_ctr != newer_pages){
        for(int i = ind0; i < master.size(); i ++){
            res.emplace_back(master[i]);
        }
        ind0 = 0;
        newer_pg_ctr++;
        if(newer_pg_ctr <= newer_pages){
            master = newer->get_pages(newer_pg_ctr, newer_pg_ctr);
        }
        // Flush page
        while(res.size() >= PAGE_NUM_ENTRIES){
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
        }
    }
    while(older_pg_ctr != older_pages){
        for(int i = ind1; i < older_page.size(); i ++){
            res.emplace_back(older_page[i]);
        }
        ind1 = 0;
        older_pg_ctr++;
        if(older_pg_ctr <= older_pages){
            older_page = older->get_pages(older_pg_ctr, older_pg_ctr);
        }
        // Flush page
        while(res.size() >= PAGE_NUM_ENTRIES){
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

    // Update metadata with new size
    meta[2] = total_size;
    fileManager->write_page(meta, PAGE_SIZE, 0, fname);
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
    delete levels[level][0];
    delete levels[level][1];
    levels[level].clear();
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
