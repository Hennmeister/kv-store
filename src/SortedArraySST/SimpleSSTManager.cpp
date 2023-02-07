#include "../../include/SimpleSSTManager.h"

//
// Created by Sambamurthy Vijay on 2023-02-06.
//

bool SimpleSSTManager::get(const int& key, int& value){
    return false;
}

int SimpleSSTManager::open(std::string &target_dir) {
    return 0;
}

bool SimpleSSTManager::add_sst(std::vector<std::pair<int, int>> data) {
    return false;
}

std::vector<std::pair<int, int>> SimpleSSTManager::scan(const int& key1, const int& key2) {
    std::vector<std::pair<int, int>> vec;
    return vec;
}

SimpleSSTManager::SimpleSSTManager() {

}
