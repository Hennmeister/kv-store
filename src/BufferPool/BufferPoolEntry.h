#ifndef KV_STORE_BUFFERPOOLENTRY_H
#define KV_STORE_BUFFERPOOLENTRY_H

#include "../../include/constants.h"
#include <stdint.h>

using namespace std;
// TODO: make this a struct?
class BufferPoolEntry {
public:
    uint32_t page_num;
    char dirty; // 1 indicates page is dirty, 0 otherwise
    uint8_t page[PAGE_SIZE]; // 4KB page
    BufferPoolEntry *next_entry;
    BufferPoolEntry *prev_entry; // for eviction
};


#endif //KV_STORE_BUFFERPOOLENTRY_H
