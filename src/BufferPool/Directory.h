#ifndef KV_STORE_DIRECTORY_H
#define KV_STORE_DIRECTORY_H

#include "vector"
#include "BufferPoolEntry.h"
#include "EvictionStrategy.h"

class Directory {
public:
    void insert_page(uint32_t page_num, void *page);
    uint8_t *get_page(uint32_t page_num); // returns the page data in entry
    void set_size(int new_size);

private:
    int min_size;
    int max_size;
    int curr_size;
    int num_bits;
    std::vector<BufferPoolEntry *> entries;
    EvictionStrategy *evictionStrategy;

    void grow();
    void shrink();
};


#endif //KV_STORE_DIRECTORY_H
