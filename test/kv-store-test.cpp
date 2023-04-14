#ifndef SRC_KV_STORE_KV_STORE_TEST_H
#define SRC_KV_STORE_KV_STORE_TEST_H

#include "../include/SimpleKVStore.h"
#include "../include/RedBlack/RedBlackMemtable.h"
#include "../include/ArraySST/SimpleSSTManager.h"
#include "../include/constants.h"
#include "../include/util.h"
#include "./test_util.h"
#include "./tests.h"
#include "../include/Base/BufferPool.h"
#include <string>
#include <vector>
#include <filesystem>
#include <sys/stat.h>
#include <iostream>
#include <random>
#include <unordered_set>

using namespace std;

string test_dir = "./test_dbs/";

#define F_NAME(func)                         \
    pair<void (*)(SimpleKVStore db), string> \
    {                                        \
        func, #func                          \
    }

vector<pair<void (*)(SimpleKVStore db), string>>
    individual_db_tests = {
        F_NAME(simple_test),
        F_NAME(hash_test),
        F_NAME(memtable_puts_and_gets),
        F_NAME(sequential_puts_and_gets),
        F_NAME(sequential_puts_and_scans),
        F_NAME(random_puts_and_gets),
        F_NAME(update_keys),
        F_NAME(edge_case_values),
        F_NAME(multiple_dbs),
        F_NAME(simple_LRU_buffer),
        F_NAME(LRU_simple_evict),
        F_NAME(LRU_ref_evict),
        F_NAME(LRU_grow),
        F_NAME(LRU_shrink),
        F_NAME(simple_clock_buffer),
        F_NAME(clock_simple_evict),
        F_NAME(bloom_filter_simple)
};

vector<pair<void (*)(SimpleKVStore db), string>> shared_db_tests = {
        F_NAME(close_and_recover)
};

vector<pair<string, DbOptions *>> get_db_options() {
    vector<pair<string, DbOptions *>> options;

    DbOptions *default_options = new DbOptions();
    options.push_back(pair("default_options", default_options));

    // Clock eviction strategy
    DbOptions *clock_options = new DbOptions();
    clock_options->setBufferPoolType("Clock");
    options.push_back(pair("clock_options", clock_options));

    //  LRU eviction strategy, large buffer pool
    DbOptions *LRU_options = new DbOptions();
    LRU_options->setBufferPoolType("LRU");
    LRU_options->setBufferPoolSize(50, 100); // 100MB
    options.push_back(pair("LRU_options", LRU_options));

    // No buffer pool
    DbOptions *no_buffer_options = new DbOptions();
    no_buffer_options->setBufferPoolType("None");
    options.push_back(pair("no_buffer_options", no_buffer_options));

    // No buffer pool and no bloom filter
    DbOptions *no_buffer_or_filter_options = new DbOptions();
    no_buffer_or_filter_options->setBufferPoolType("None");
    no_buffer_or_filter_options->setFilterBitsPerEntry(0);
    options.push_back(pair("no_buffer_or_filter_options", no_buffer_or_filter_options));

    // Binary search SST
    DbOptions *binary_search_options = new DbOptions();
    binary_search_options->setSSTSearch("Binary Search");
    options.push_back(pair("binary_search_options", binary_search_options));

    // large memtable and bloom filters
    DbOptions *large_memtable_and_filter_options = new DbOptions();
    large_memtable_and_filter_options->setMaxMemtableSize(99999999);
    large_memtable_and_filter_options->setFilterBitsPerEntry(1000);
    options.push_back(pair("large_memtable_and_filter_options", large_memtable_and_filter_options));

    // small btree fanout
    DbOptions *small_btree_fanout_options = new DbOptions();
    small_btree_fanout_options->maxMemtableSize = 100000;
    small_btree_fanout_options->setBtreeFanout(2);
    options.push_back(pair("small_btree_fanout_options", small_btree_fanout_options));

    // large btree fanout
    DbOptions *large_btree_fanout_options = new DbOptions();
    large_btree_fanout_options->maxMemtableSize = 10000;
    large_btree_fanout_options->setBtreeFanout(1000);
    options.push_back(pair("large_btree_fanout_options", large_btree_fanout_options));

    return options;
}

int main()
{
    cout << endl
         << "🧪 Running SimpleKVStore Tests 🧪" << endl
         << endl;

    if (dir_exists(test_dir) == 0)
        mkdir(test_dir.c_str(), 0777);
    else if (dir_exists(test_dir) == 1)
    {
        for (const auto &entry : std::filesystem::directory_iterator(test_dir))
            std::filesystem::remove_all(entry.path());
    }

    //       Individual DBs
    vector<pair<string, DbOptions *>> db_options = get_db_options();
    for (auto options : db_options) {
        cout << "////// Running with options: " + options.first << " //////" << endl;
        for (pair<void (*)(SimpleKVStore db), string> func: individual_db_tests) {
            // Before all
            SimpleKVStore db;
            db.open(test_dir + options.first + "_" + func.second + "_db", options.second);

            // Call method
            cout << func.second;
            func.first(db);
            cout << " ✅" << endl;

            // After all
            db.close();
        }

    }

    // Shared DBs
    SimpleKVStore shared_db;
    shared_db.open(test_dir + "shared_db");

    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++)
    {
        shared_db.put(i, -i);
    }

    for (pair<void (*)(SimpleKVStore db), string> func : shared_db_tests)
    {
        cout << func.second;
        func.first(shared_db);
        cout << " ✅" << endl;
    }

    cout << endl
         << "All tests passed ✅" << endl
         << endl;

    // Clear testing data
    for (const auto &entry : std::filesystem::directory_iterator(test_dir))
        std::filesystem::remove_all(entry.path());

    return 0;
}

#endif // SRC_KV_STORE_KV_STORE_TEST_H
