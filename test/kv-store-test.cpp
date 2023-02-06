//
// Created by Henning Lindig on 2023-01-27.
//

#ifndef SRC_KV_STORE_KV_STORE_TEST_H
#define SRC_KV_STORE_KV_STORE_TEST_H

#include <iostream>
#include <cassert>
#include "../include/kv-store.h"

using namespace std;

int main()
{
    cout << "Running KVStore Tests..."<< endl;

    KVStore db;
    db.open("new_db");
    db.put(1,1);
    db.put(-2, -2);
    db.put(5,5);
    assert(db.get(1) == 1);
    assert(db.get(-2) == -2);
    assert(db.get(5) == 5);
    assert(db.get(-1) == 0);
    db.put(1, 10);
    assert(db.get(1) == 10);
    assert(db.scan(1, 7).size() == 2);
    assert(db.scan(-20, 20).size() == 3);
    db.put(1000,7);
    db.put(1001,8);
    db.put(1002,9);
    db.put(1003,10);
    db.close();

    cout << "All tests passed" << endl;

    return 0;
}

#endif //SRC_KV_STORE_KV_STORE_TEST_H
