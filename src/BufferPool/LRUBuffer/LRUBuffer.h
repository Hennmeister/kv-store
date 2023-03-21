#ifndef KV_STORE_LRUBUFFER_H
#define KV_STORE_LRUBUFFER_H

#include "LRUBufferEntry.h"
#include "../Directory.h"
#include "LRUNode.h"

class LRUBuffer: public Directory<LRUBufferEntry> {
public:
    LRUBuffer(int minSize, int maxSize);
    void put(int page_num, std::uint8_t page[PAGE_SIZE]) override;
    int get(int page_num, std::uint8_t page_out_buf[PAGE_SIZE]) override;

private:
    LRUNode *head;
    LRUNode *tail;
};

#endif //KV_STORE_LRUBUFFER_H
