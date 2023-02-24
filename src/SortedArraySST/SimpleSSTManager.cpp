#include <iostream>
#include <sys/stat.h>
#include <filesystem>
#include <utility>
#include "../../include/SimpleSSTManager.h"

namespace fs = std::filesystem;

char* StringToChar(std::string str){
    const int length = str.length();

    // declaring character array (+1 for null terminator)
    char* char_array = new char[length + 1];

    // copying the contents of the
    // string to char array
    strcpy(char_array, str.c_str());
    return char_array;
}

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
    for(SortedArraySST sst: SSTs ){
        if(sst.get(key,value)){
            return true;
        }
    }
    return false;
}

bool SimpleSSTManager::add_sst(std::vector<std::pair<int, int>> data) {
    SSTs.push_back(*new SortedArraySST(directory, data));
    return true;
}

std::vector<std::pair<int, int>> SimpleSSTManager::scan(const int& key1, const int& key2) {
    std::vector<std::pair<int, int>> vec;
    return vec;
}

SimpleSSTManager::SimpleSSTManager(std::string target_dir) {
    directory = std::move(target_dir);
    SSTs = {};
    // TODO: Update SSTManager and SST class with single file structure.
    char* dir_char = StringToChar(directory);
    int dir = DirectoryExists(dir_char);
    if(dir == 0){
        mkdir(dir_char, 0777);
        delete[] dir_char;
    }else if(dir == 1){
        for (const auto & entry : fs::directory_iterator(directory))
            SSTs.push_back(*new SortedArraySST(entry));
        delete[] dir_char;
    }else{
        throw std::invalid_argument("Provided path is a file - unable to initialize SSTManager.");
    }
}

void SimpleSSTManager::delete_data() {
    std::error_code errorCode;
    if (!fs::remove_all(directory, errorCode)) {
        std::cout << errorCode.message() << std::endl;
    }
}

