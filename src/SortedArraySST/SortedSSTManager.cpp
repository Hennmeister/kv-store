
#include <fcntl.h>
#include "../../include/SortedSSTManager.h"
#include "../../include/util.h"
#include <unistd.h>


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
    char* index_file = string_to_char(prefix + "_index.sdb");
    char* sst_file = string_to_char(prefix + "_index.sdb");

    int index_file_fd = open(index_file, O_RDWR | O_CREAT);
    int sst_file_fd = open(sst_file, O_RDWR | O_CREAT);


}

SortedSSTManager::~SortedSSTManager() {

}
