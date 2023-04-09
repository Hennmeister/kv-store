#include <iostream>
#include <string>
#include <filesystem>
#include "./include/SimpleKVStore.h"

using namespace std;

string path = "/playground_db/";

int main()
{
    SimpleKVStore db;
    db.open(path + "db1");

    int key = 1; 
    int val = 1000;

    if (db.put(key, val)) 
        std::cout << "Put suceeded" << std::endl;
    else 
        std::cout << "Put failed" << std::endl;

    int query;
    if (db.get(key, query)) 
        std::cout << "Get suceeded: " + to_string(query) << std::endl;
    else 
        std::cout << "Get failed" << std::endl;

    db.close();

    // Clear playground data
    for (const auto &entry : std::filesystem::directory_iterator(path))
        std::filesystem::remove_all(entry.path());

    return 0;
}