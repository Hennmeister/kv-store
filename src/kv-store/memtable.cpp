#include "../../include/memtable.h"
#include "../../include/red-black-tree.h"

#include <iostream>
#include <fstream>

using namespace std;

Memtable::Memtable(const int& memtable_size, const string& directory) {
    capacity = memtable_size;
    this->directory = directory;
    data = new RedBlackTree();
    sst_size = 0;
}

void Memtable::put(const int &key, const int &value) {
    if (data->getSize() >= capacity && get(key) == NULL) {
        if (!dumpToSst()) {
            cout << "Invalid file" << endl;
        }
        data = new RedBlackTree();
        sst_size++;
    }
    data->put(key, value);
}

int Memtable::get(const int &key) {
    return data->get(key);
}

vector<pair<int, int>> Memtable::scan(const int &key1, const int &key2) {
    return data->scan(key1, key2);
}

vector<pair<int, int>> Memtable::inorderTraversal() {
    return data->inorderTraversal();
}

bool Memtable::dumpToSst() {
    ofstream *file = new ofstream();
    file->open(this->directory + "/ssts.txt", ios::binary | ios::app);

    if (!file->is_open())
    {
        return false;
    }

    (* file) << data->getSize() << endl;

    for (pair<int,int> pair : inorderTraversal()) {
        (* file) << pair.first << "," << pair.second << endl;
    }

    file->close();

    sst_size++;

    return true;
}