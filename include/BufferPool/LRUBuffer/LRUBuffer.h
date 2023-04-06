#ifndef KV_STORE_LRUBUFFER_H
#define KV_STORE_LRUBUFFER_H

#include "LRUBufferEntry.h"
#include "../Directory.h"
#include "LRUNode.h"
#include <stdint.h>
#include <cstring>
#include <set>

// An implementation of an extendable hashing directory employing an LRU eviction policy
class LRUBuffer: public Directory<LRUBufferEntry> {
public:
    LRUBuffer(int minSize, int maxSize);
    void put(int page_num, std::uint8_t page[PAGE_SIZE]) override;
    int get(int page_num, std::uint8_t page_out_buf[PAGE_SIZE]) override;
    void evict() override;

private:
    LRUNode *head;
    LRUNode *tail;
};

#endif //KV_STORE_LRUBUFFER_H
