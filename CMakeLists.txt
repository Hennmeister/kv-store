# CMakeList.txt : CMake project for SimpleKVStore, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

project ("kv-store")

add_executable (kv-store-test test/kv-store-test.cpp test/kv-store-test.h test/test_util.cpp test/test_util.h test/tests.cpp test/tests.h)

add_library(kv-store src/KVStore/SimpeKVStore.cpp include/Base/KVStore.h include/RedBlackTree.h
        src/RedBlackTree/RedBlackTree.cpp src/KVStore/RedBlackMemtable.cpp include/RedBlackMemtable.h
        include/Base/KVStore.h include/SimpleKVStore.h include/Base/Memtable.h include/Base/SSTManager.h
        include/SimpleSSTManager.h include/Base/SST.h src/SortedArraySST/SimpleSSTManager.cpp include/constants.h
        include/SortedSSTManager.h src/SortedArraySST/SortedSSTManager.cpp include/util.h src/util/util.cpp src/BufferPool/BufferPoolEntry.cpp src/BufferPool/BufferPoolEntry.h src/BufferPool/Directory.cpp src/BufferPool/Directory.h src/BufferPool/EvictionStrategy.h src/util/MurmurHash3/MurmurHash3.cpp src/util/MurmurHash3/MurmurHash3.h)
# Add source to this project's executable.

target_link_libraries(kv-store-test PUBLIC kv-store)