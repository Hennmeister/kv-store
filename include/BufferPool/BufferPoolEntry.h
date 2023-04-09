#ifndef KV_STORE_BUFFERPOOLENTRY_H
#define KV_STORE_BUFFERPOOLENTRY_H

#include "../constants.h"
#include <stdint.h>
#include <string>

using namespace std;

template <typename T>
struct BufferPoolEntry {
    std::string file_and_page;
    uint8_t page[PAGE_SIZE];
    T *prev_entry;
    T *next_entry;
};


#endif //KV_STORE_BUFFERPOOLENTRY_H
