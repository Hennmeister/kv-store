
#include <fcntl.h>
#include "../../include/SortedSSTManager.h"


bool SortedSSTManager::get(const int &key, int &value) {
    return false;
}

bool SortedSSTManager::add_sst(std::vector<std::pair<int, int>> data) {
    return false;
}

std::vector<std::pair<int, int>> SortedSSTManager::scan(const int &key1, const int &key2) {
    return std::vector<std::pair<int, int>>();
}

SortedSSTManager::SortedSSTManager(std::string prefix) {
//    int index_file_fd = open((char *)prefix + "_index.sdb", O_RDWR | O_CREAT);

}
