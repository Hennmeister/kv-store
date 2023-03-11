#ifndef KV_STORE_DIRECTORY_H
#define KV_STORE_DIRECTORY_H

#include "vector"
#include "BufferPoolEntry.h"
#include "EvictionStrategy.h"
#include "BufferPool.h"

class Directory: public BufferPool {
public:
    Directory(int min_size, int max_size, EvictionStrategy *eviction_strategy);

    void insert_page(uint32_t page_num, ::uint8_t *page) override;
    int get_page(uint32_t page_num, uint8_t *page_out_buf) override; // returns the page data in entry
    void set_size(int new_size) override;

private:
    int min_size;
    int max_size;
    int num_bits;
    float num_pages_in_buffer;
    std::vector<BufferPoolEntry *> entries;
    EvictionStrategy *eviction_strategy;

    void grow(int new_num_bits);
    void shrink(int new_num_bits);
    void insert(BufferPoolEntry *entry); // used internally for moving around entries
    ::uint32_t hash_to_bucket_index(::uint32_t page_num) const;
};


#endif //KV_STORE_DIRECTORY_H
