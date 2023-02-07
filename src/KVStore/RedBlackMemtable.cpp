#include "../../include/RedBlackMemtable.h"
#include "../../include/RedBlackTree.h"

#include <iostream>
#include <fstream>

using namespace std;
RedBlackMemtable::RedBlackMemtable(){
    this->data = new RedBlackTree();
}

void RedBlackMemtable::put(const int &key, const int &value)  {
    data->put(key, value);
    size++;
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

bool RedBlackMemtable::reset() {
    return false;
}

//bool RedBlackMemtable::dumpToSst() {
//    auto *file = new ofstream();
//    file->open( "./ssts.txt", ios::binary | ios::app);
//
//    if (!file->is_open())
//    {
//        return false;
//    }
//
//    (* file) << data->getSize() << endl;
//
//    for (pair<int,int> pair : inorderTraversal()) {
//        (* file) << pair.first << "," << pair.second << endl;
//    }
//
//    file->close();
//
//    return true;
//}
//








