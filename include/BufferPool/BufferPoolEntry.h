#ifndef KV_STORE_BUFFERPOOLENTRY_H
#define KV_STORE_BUFFERPOOLENTRY_H

#include "../constants.h"
#include <stdint.h>
#include <string>

using namespace std;

template <typename T>
struct BufferPoolEntry {
    std::string file_and_page;
    // entry_data holds page_size amount of data
    uint8_t *entry_data;
    int page_size;
    T *prev_entry;
    T *next_entry;
};


#endif //KV_STORE_BUFFERPOOLENTRY_H
