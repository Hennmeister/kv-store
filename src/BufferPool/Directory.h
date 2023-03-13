#ifndef KV_STORE_DIRECTORY_H
#define KV_STORE_DIRECTORY_H

#include "vector"
#include "BufferPoolEntry.h"
#include "EvictionStrategy.h"
#include "BufferPool.h"
#include <iostream>

class Directory: public BufferPool {
public:
    friend std::ostream& operator<<(std::ostream& out, const Directory& dirBufferPool) {
        out << "Printing Directory" << endl;
        out << "num pages: " << dirBufferPool.num_pages_in_buffer << ", num_bits: " << dirBufferPool.num_bits <<  endl;
        BufferPoolEntry *curr_entry;
        for (int i = 0; i < dirBufferPool.entries.size(); i++) {
            out << i << ": ";
            curr_entry = dirBufferPool.entries[i];
            while (curr_entry != nullptr) {
                out << curr_entry->page_num << ", ";
                curr_entry = curr_entry->next_entry;
            }
        }
    }

    Directory(int min_size, int max_size, EvictionStrategy *eviction_strategy);

    void insert_page(uint32_t page_num, uint8_t page[PAGE_SIZE]) override;
    int get_page(uint32_t page_num, uint8_t page_out_buf[PAGE_SIZE]) override; // returns the page data in entry
    void set_size(int new_size) override;

private:
    int min_size;
    int max_size;
    int num_bits;
    int num_pages_in_buffer;
    EvictionStrategy *eviction_strategy;
    std::vector<BufferPoolEntry *> entries;

    void grow(int new_num_bits);
    void shrink(int new_num_bits);
    void insert(BufferPoolEntry *entry); // used internally for moving around entries
    ::uint32_t hash_to_bucket_index(::uint32_t page_num) const;
};


#endif //KV_STORE_DIRECTORY_H
