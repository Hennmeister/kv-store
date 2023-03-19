#include "../../include/BTree/BTreeSST.h"

BTreeSST::~BTreeSST() {

}

int BTreeSST::getSize() {
    return 0;
}

BTreeSST::BTreeSST(SSTFileManager *fileManager, int ind, int fanout, vector<pair<int, int>> data, int useBinarySearch) {

}

BTreeSST::BTreeSST(SSTFileManager *fileManager, int ind, string filename, int useBinarySearch) {

}

bool BTreeSST::get(const int &key, int &value) {
    return false;
}

std::vector<std::pair<int, int>> BTreeSST::scan(const int &key1, const int &key2) {
    return std::vector<std::pair<int, int>>();
}
