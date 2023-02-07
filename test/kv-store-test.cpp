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

using namespace std;

int main()
{
    cout << "Running SimpleKVStore Tests..."<< endl;

    SimpleKVStore db;
    db.open("new_db", new RedBlackMemtable(), 5, new SimpleSSTManager( "my_db"));
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
    assert(val == 10);
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
