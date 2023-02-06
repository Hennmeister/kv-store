#ifndef KV_STORE_MEMTABLE_H
#define KV_STORE_MEMTABLE_H

#include <tuple>
#include <string>
#include <vector>
#include "memtable-data.h"

using namespace std;

class Memtable {
private:
    int capacity;
    string directory;
    MemtableData *data;
    int sst_size;
public:
    Memtable(const int& memtable_size, const string& directory);

    void put(const int& key, const int& value);
    int get(const int& key);
    vector<pair<int, int>> scan(const int& key1, const int& key2);
    vector<pair<int, int>> inorderTraversal();
    bool dumpToSst();
};

#endif //KV_STORE_MEMTABLE_H
