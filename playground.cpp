#include <iostream>
#include <string>
#include <filesystem>
#include "./include/SimpleKVStore.h"

using namespace std;

int main()
{
    SimpleKVStore db;
    db.open("playground");

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

    std::filesystem::remove_all("/playground");

    return 0;
}