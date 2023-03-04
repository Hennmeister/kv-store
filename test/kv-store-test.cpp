//
// Created by Henning Lindig on 2023-01-27.
//

#ifndef SRC_KV_STORE_KV_STORE_TEST_H
#define SRC_KV_STORE_KV_STORE_TEST_H

#include <iostream>
#include <cassert>
#include "../include/SimpleKVStore.h"
#include "../include/RedBlackMemtable.h"
#include "../include/SimpleSSTManager.h"
#include "../include/constants.h"
#include "../include/util.h"
#include "./test_util.h"
#include <string>
#include <vector>
#include <filesystem>
#include <sys/stat.h>

using namespace std;

// ===================== Inner-workings Tests =========================

void memtable_puts_and_gets(SimpleKVStore db) {

}

void memtable_puts_and_scans(SimpleKVStore db) {

}

// ===================== User-facing Tests =========================

void sequeantial_puts_and_gets(SimpleKVStore db) {

    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++) {
       db.put(i, -i);
    }

    int val;
    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++) {
        db.get(i, val);
        assert_val_equals(val, -i, "sequeantial_puts_and_gets");
    }
}

void sequeantial_puts_and_scans(SimpleKVStore db) {

    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++) {
       db.put(i, -i);
    }

    vector<pair<int, int>> key_vals{}; 
    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++) {
        key_vals.push_back(pair<int,int>({i, -i}));
        vector<pair<int, int>> scan = db.scan(0, i);
        assert_vec_equals(scan, key_vals, "sequeantial_puts_and_scans_1");
    }

    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++) {
        vector<pair<int, int>> scan = db.scan(i, 3 * PAGE_NUM_ENTRIES + 300 - 1);
        assert_vec_equals(scan, key_vals, "sequeantial_puts_and_scans_2");
        key_vals.erase(key_vals.begin());
    }
}

void update_keys(SimpleKVStore db) {

}

void edge_case_values(SimpleKVStore db) {
    // assert_vec_equals(db.scan(5, 4), vector<pair<int, int>>{}, "edge_case_values");
}

void simple_test(SimpleKVStore db) {
    db.put(1, 1);
    db.put(-2, -2);
    db.put(5,5);
    int val = 0;
    db.get(1,val);
    assert(val == 1);
    db.get(-2, val);
    assert(val == -2);
    db.get(5,val);
    assert(val == 5);
    assert(db.get(-1,val) == false);
    db.put(1, 10);
    db.get(1, val);
    assert(val == 10); 
}

#define F_NAME(func)  pair<void (*) (SimpleKVStore db), string> {func, #func}

vector<pair<void (*) (SimpleKVStore db), string>> testing_suite = {
    F_NAME(simple_test),
    F_NAME(memtable_puts_and_gets), 
    F_NAME(memtable_puts_and_scans),
    F_NAME(sequeantial_puts_and_gets),
    F_NAME(sequeantial_puts_and_scans),
    F_NAME(update_keys),
    F_NAME(edge_case_values)
};

string target_dir = "./test_dbs/";

int main()
{
    cout << "Running SimpleKVStore Tests..."<< endl << endl;

    if(dir_exists(target_dir) == 0)
        mkdir(target_dir.c_str(), 0777);
    else if (dir_exists(target_dir) == 1) {
        for (const auto& entry : std::filesystem::directory_iterator(target_dir))
            std::filesystem::remove_all(entry.path());
    }

    for (pair<void (*) (SimpleKVStore db), string> func: testing_suite) {

        // Before all

        SimpleKVStore db;
        db.open(target_dir + func.second + "_db", PAGE_NUM_ENTRIES);

        // Call method

        cout << func.second;
        func.first(db);
        cout << " ✅" << endl;

        // After all
        db.close();
    }

    cout << endl << "All tests passed" << endl;

    // Clear testing data
    for (const auto& entry : std::filesystem::directory_iterator(target_dir))
            std::filesystem::remove_all(entry.path());

    return 0;
}

#endif //SRC_KV_STORE_KV_STORE_TEST_H
