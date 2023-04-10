#ifndef KV_STORE_BUFFERPOOL_H
#define KV_STORE_BUFFERPOOL_H
#include "../constants.h"
#include <stdint.h>
#include <string>

class BufferPool {
public:
    virtual bool put(std::string file_and_page, uint8_t *data, int size) = 0;
    virtual bool get(std::string file_and_page, uint8_t page_out_buf[PAGE_SIZE]) = 0;
    virtual void set_max_size(int new_size) = 0;
    virtual ~BufferPool() = default;
};

#endif //KV_STORE_BUFFERPOOL_H
