#ifndef KV_STORE_BUFFERPOOLENTRY_H
#define KV_STORE_BUFFERPOOLENTRY_H

#include "../../include/constants.h"
#include <stdint.h>

using namespace std;

template <typename  T>
struct BufferPoolEntry {
    uint32_t page_num;
    uint8_t page[PAGE_SIZE];
    T *prev_entry;
    T *next_entry;
};


#endif //KV_STORE_BUFFERPOOLENTRY_H
