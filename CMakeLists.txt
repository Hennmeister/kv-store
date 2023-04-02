# CMakeList.txt : CMake project for SimpleKVStore, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

project ("kv-store")

add_executable (kv-store-test test/kv-store-test.cpp test/kv-store-test.h test/test_util.cpp test/test_util.h test/tests.cpp test/tests.h)

add_library(kv-store src/KVStore/SimpeKVStore.cpp include/Base/KVStore.h include/RedBlack/RedBlackTree.h
        src/RedBlackTree/RedBlackTree.cpp src/KVStore/RedBlackMemtable.cpp include/RedBlack/RedBlackMemtable.h
        include/Base/KVStore.h include/SimpleKVStore.h include/Base/Memtable.h include/Base/SSTManager.h
        include/ArraySST/SimpleSSTManager.h include/Base/SST.h include/constants.h
        include/ArraySST/SortedSSTManager.h src/SortedArraySST/SortedSSTManager.cpp include/util.h src/util.cpp include/Base/SSTFileManager.h include/SimpleSSTFileManager.h src/SimpleSSTFileManager.cpp include/BTree/BTreeSSTManager.h include/BTree/BTreeSST.h src/BTree/BTreeSSTManager.cpp src/BTree/BTreeSST.cpp include/LSMTreeManager.h src/LSMTreeManager.cpp)
# Add source to this project's executable.

target_link_libraries(kv-store-test PUBLIC kv-store)