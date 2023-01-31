# CMakeList.txt : CMake project for kv-store, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

project ("kv-store")

add_executable (kv-store-test test/kv-store-test.cpp test/kv-store-test.h)

add_library(kv-store src/kv-store/kv-store.cpp include/kv-store.h src/red-black-tree/red-black-tree.h src/red-black-tree/red-black-tree.cpp src/kv-store/memtable.h)
# Add source to this project's executable.

target_link_libraries(kv-store-test PUBLIC kv-store)