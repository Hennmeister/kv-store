#include "../../include/RedBlackMemtable.h"
#include "../../include/RedBlackTree.h"

#include <iostream>
#include <fstream>

using namespace std;
RedBlackMemtable::RedBlackMemtable(const int& memtable_size){
    capacity = memtable_size;
    this->data = new RedBlackTree();
}

void RedBlackMemtable::put(const int &key, const int &value)  {
    int val; // Unused store
    if (data->getSize() >= capacity && !get(key, val)) {
        if (!dumpToSst()) {
            cout << "Invalid file" << endl;
        }
        data = new RedBlackTree();
    }
    data->put(key, value);
}

bool RedBlackMemtable::get(const int &key, int& value)  {
    return data->get(key, value);
}

vector<pair<int, int>> RedBlackMemtable::scan(const int &key1, const int &key2)  {
    return data->scan(key1, key2);
}

vector<pair<int, int>> RedBlackMemtable::inorderTraversal()  {
    return data->inorderTraversal();
}

bool RedBlackMemtable::dumpToSst() {
    auto *file = new ofstream();
    file->open( "./ssts.txt", ios::binary | ios::app);

    if (!file->is_open())
    {
        return false;
    }

    (* file) << data->getSize() << endl;

    for (pair<int,int> pair : inorderTraversal()) {
        (* file) << pair.first << "," << pair.second << endl;
    }

    file->close();

    return true;
}









