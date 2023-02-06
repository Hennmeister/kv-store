//
// Created by vijay on 6/2/2023.
//

#ifndef KV_STORE_REDBLACKMEMTABLE_H
#define KV_STORE_REDBLACKMEMTABLE_H

#include <string>
#include <vector>
#include "Base/Memtable.h"
#include "RedBlackTree.h"

using namespace std;

class RedBlackMemtable: public Memtable {
private:
    int capacity;
    RedBlackTree *data;
    string directory;
public:
    explicit RedBlackMemtable(const int &memtable_size);
    void put(const int& key, const int& value) override;
    bool get(const int& key, int& value) override;
    vector<pair<int, int>> scan(const int& key1, const int& key2) override;
    vector<pair<int, int>> inorderTraversal() override;

    bool dumpToSst();
};

#endif //KV_STORE_REDBLACKMEMTABLE_H
