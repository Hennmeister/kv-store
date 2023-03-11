#ifndef KV_STORE_BUFFERPOOL_H
#define KV_STORE_BUFFERPOOL_H

#include <cstdint>

class BufferPool {
public:
    // TODO: overload [] and set operators instead
    virtual void insert_page(uint32_t page_num, ::uint8_t *page) = 0;
    virtual int get_page(uint32_t page_num, uint8_t *page_out_buf) = 0; // returns the page data in entry
    virtual void set_size(int new_size) = 0;
    virtual ~BufferPool() = default;
};

#endif //KV_STORE_BUFFERPOOL_H
