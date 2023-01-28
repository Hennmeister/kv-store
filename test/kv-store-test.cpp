//
// Created by Henning Lindig on 2023-01-27.
//

#ifndef SRC_KV_STORE_KV_STORE_TEST_H
#define SRC_KV_STORE_KV_STORE_TEST_H

#include <iostream>
#include "../include/kv-store.h"

using namespace std;

int main()
{
    cout << "Running KVStore Tests "<< endl;
    RedBlackTree *memtable = new RedBlackTree(0, 0);
    memtable->insert(1, 5);
    memtable->insert(2, 4);
    memtable->insert(-1, 5);
    cout << endl << memtable->get(1) << memtable->get(2) << memtable->get(-1) << memtable->get(5) << endl;

    cout << "traversal: " << endl;
    std::vector<std::pair<int, int>> arr = memtable->scan(-20, 20);


    return 0;
}

#endif //SRC_KV_STORE_KV_STORE_TEST_H
