#include <iostream>
#include <sys/stat.h>
#include "../../include/SimpleSSTManager.h"

//
// Created by Sambamurthy Vijay on 2023-02-06.
//

int DirectoryExists(char* dir){
    struct stat info{};
    if( stat( dir, &info ) != 0 ){
        return 0;
    }
    else if( info.st_mode & S_IFDIR )
    {
        return 1;
    }
    return -1;
}


bool SimpleSSTManager::get(const int& key, int& value){
    return false;
}

bool SimpleSSTManager::add_sst(std::vector<std::pair<int, int>> data) {
    std::cout << "adding sst" << std::endl;
    return false;
}

std::vector<std::pair<int, int>> SimpleSSTManager::scan(const int& key1, const int& key2) {
    std::vector<std::pair<int, int>> vec;
    return vec;
}

SimpleSSTManager::SimpleSSTManager(char* target_dir) {
    int dir = DirectoryExists(target_dir);
    if(dir == 0){
        mkdir(target_dir, 0777);
    }else if(dir == 1){
        std::cout << "dir does exist" << std::endl;
    }else{
        throw std::invalid_argument("Provided path is a file.");
    }
}

