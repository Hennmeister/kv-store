#include <vector>
#include "../../include/SortedArraySST.h"
#include <iostream>
#include <fstream>
#include <string>
//
// Created by Sambamurthy Vijay on 2023-02-06.
//
bool SortedArraySST::get(const int &key, int& value) {
    for (std::pair<int,int> pair : data) {
        if(pair.first == key){
            value = pair.second;
            return true;
        }
    }
    return false;
}

std::vector<std::pair<int, int>> SortedArraySST::scan(const int &key1, const int &key2) {
    std::vector<std::pair<int, int>> vec;
    return vec;
}

SortedArraySST::SortedArraySST(const std::filesystem::directory_entry file) {
    // TODO: Update SSTManager and SST class with single file structure.
    std::ifstream read_file(file.path());
    std::string text;
    std::getline(read_file,text);

    size = std::stoi(text);
    this->data = {};
    for (int i =0; i < size;i++) {
        std::getline(read_file,text);
        int k = std::stoi(text.substr(0, text.find(",")));
        int v = std::stoi(text.substr(text.find(",") + 1, text.size()));
        data.emplace_back(k, v);
    }
    read_file.close();
}

SortedArraySST::SortedArraySST(std::string& fname, const std::vector<std::pair<int, int>>& data) {
    // TODO: Update SSTManager and SST class with single file structure.
    size = static_cast<int>(data.size());
    auto *write_file = new std::ofstream();
    write_file->open(fname);

    (* write_file) << size << std::endl;
    this->data = data;
    for (std::pair<int,int> pair : data) {
        (* write_file) << pair.first << "," << pair.second << std::endl;
    }
    write_file->close();
}

