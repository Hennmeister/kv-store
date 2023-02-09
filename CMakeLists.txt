# CMakeList.txt : CMake project for SimpleKVStore, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

project ("kv-store")

add_executable (kv-store-test test/kv-store-test.cpp test/kv-store-test.h)

add_library(kv-store src/KVStore/SimpeKVStore.cpp include/Base/KVStore.h include/RedBlackTree.h src/RedBlackTree/RedBlackTree.cpp src/KVStore/RedBlackMemtable.cpp include/RedBlackMemtable.h include/Base/KVStore.h include/SimpleKVStore.h include/Base/Memtable.h include/Base/SSTManager.h include/SimpleSSTManager.h include/Base/SST.h src/SortedArraySST/SimpleSSTManager.cpp include/SortedArraySST.h src/SortedArraySST/SortedArraySST.cpp)
# Add source to this project's executable.

target_link_libraries(kv-store-test PUBLIC kv-store)