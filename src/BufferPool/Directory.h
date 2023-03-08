#ifndef KV_STORE_DIRECTORY_H
#define KV_STORE_DIRECTORY_H

#include "vector"
#include "BufferPoolEntry.h"
#include "EvictionStrategy.h"

class Directory {
public:
    Directory(int min_size, int max_size);
    void insert_page(uint32_t page_num, ::uint8_t *page);
    int get_page(uint32_t page_num, uint8_t *page_out_buf); // returns the page data in entry
    void set_size(int new_size);

private:
    int min_size;
    int max_size;
    int num_bits;
    std::vector<BufferPoolEntry *> entries;
    EvictionStrategy *evictionStrategy;

    void grow();
    void shrink();
    ::uint32_t hash_to_bucket_index(::uint32_t page_num);
};


#endif //KV_STORE_DIRECTORY_H
