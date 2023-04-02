# CMakeList.txt : CMake project for SimpleKVStore, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

project ("kv-store")

add_library(kv-store src/KVStore/SimpleKVStore.cpp include/Base/KVStore.h include/RedBlackTree.h
src/RedBlackTree/RedBlackTree.cpp src/KVStore/RedBlackMemtable.cpp include/RedBlackMemtable.h
include/Base/KVStore.h include/SimpleKVStore.h include/Base/Memtable.h include/Base/SSTManager.h
include/SimpleSSTManager.h include/Base/SST.h src/SortedArraySST/SimpleSSTManager.cpp include/constants.h
include/SortedSSTManager.h src/SortedArraySST/SortedSSTManager.cpp include/util.h src/util.cpp)
# Add source to this project's executable.

add_executable (kv-store-test test/kv-store-test.cpp test/kv-store-test.h test/test_util.cpp test/test_util.h test/tests.cpp test/tests.h)

target_link_libraries(kv-store-test PUBLIC kv-store)

add_executable(kv-store-performance-test experiments/kv-store-performance-testing.cpp experiments/kv-store-performance-testing)

target_link_libraries(kv-store-performance-test PUBLIC kv-store) 