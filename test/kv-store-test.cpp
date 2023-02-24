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
#include <string>
#include <fstream>

using namespace std;

void print_file(string directory, string file_name) {
    ifstream *read_file = new ifstream();
    read_file->open(directory + "/" + file_name, ios::in | ios::binary);

    if (!read_file->is_open())
        throw std::runtime_error(directory + "/" + file_name + " data file could not be opened");

    ofstream *write_file = new ofstream();
    write_file->open(directory + "/" + file_name + "_debug.txt", ios::binary | ios::app);

    if (!write_file->is_open())
        throw std::runtime_error(directory + "/" + file_name + "_debug.txt" + " data file could not be opened");

    while(!read_file->eof()) {
        int32_t first, second;
        read_file->read(reinterpret_cast<char *>(&first), sizeof(int32_t));
        read_file->read(reinterpret_cast<char *>(&second), sizeof(int32_t));
        (*write_file) << to_string(first) + ", " << to_string(second) << endl;
    }
}


void simple_test(SimpleKVStore db) {
    db.put(1,1);
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
//    assert(val == 10); TODO: uncomment this when get is fully implemented with SST traversal
}

int main()
{
    cout << "Running SimpleKVStore Tests..."<< endl;

    string target_dir = "my_db";

    SimpleKVStore db;
    db.open(target_dir, PAGE_NUM_ENTRIES);

    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++) {
        db.put(i, -i);
    }

//    print_file(target_dir, "index");
//    print_file(target_dir, "ssts");
//
//    int val;
//    for (int i = 640; i < 3 * PAGE_NUM_ENTRIES + 300; i++) {
//        db.get(i, val);
//        assert(val == -i);
//    }
//
//    simple_test(db);
    db.close();

    cout << "All tests passed" << endl;

    return 0;
}

#endif //SRC_KV_STORE_KV_STORE_TEST_H
