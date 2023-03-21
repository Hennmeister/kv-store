#ifndef KV_STORE_BUFFERPOOL_H
#define KV_STORE_BUFFERPOOL_H
#include "../../include/constants.h"

class BufferPool {
public:
    // TODO: overload [] and set operators instead
    virtual void put(int page_num, std::uint8_t page[PAGE_SIZE]) = 0;
    virtual int get(int page_num, std::uint8_t page_out_buf[PAGE_SIZE]) = 0;
    virtual void set_size(int new_size) = 0;
    virtual ~BufferPool() = default;
};

#endif //KV_STORE_BUFFERPOOL_H
