#ifndef SRC_KV_STORE_KV_STORE_TEST_H
#define SRC_KV_STORE_KV_STORE_TEST_H

#include <iostream>
#include "../include/SimpleKVStore.h"
#include "../include/RedBlackMemtable.h"
#include "../include/SimpleSSTManager.h"
#include "../include/constants.h"
#include "../include/util.h"
#include "./test_util.h"
#include "./tests.h"
#include "../src/BufferPool/BufferPool.h"
#include <string>
#include <vector>
#include <filesystem>
#include <sys/stat.h>

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
        F_NAME(memtable_puts_and_gets),
        F_NAME(memtable_puts_and_scans),
        F_NAME(sequential_puts_and_gets),
        F_NAME(sequential_puts_and_scans),
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
};

vector<pair<void (*)(SimpleKVStore db), string>> shared_db_tests = {
    F_NAME(close_and_recover)};

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

    // Individual DBs
    for (pair<void (*)(SimpleKVStore db), string> func : individual_db_tests)
    {

        // Before all

        SimpleKVStore db;
        db.open(test_dir + func.second + "_db", PAGE_NUM_ENTRIES);

        // Call method

        cout << func.second;
        func.first(db);
        cout << " ✅" << endl;

        // After all
        db.close();
    }

    // Shared DBs
    SimpleKVStore shared_db;
    shared_db.open(test_dir + "shared_db", PAGE_NUM_ENTRIES);

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
