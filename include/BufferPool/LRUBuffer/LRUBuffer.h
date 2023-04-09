#ifndef KV_STORE_LRUBUFFER_H
#define KV_STORE_LRUBUFFER_H

#include "LRUBufferEntry.h"
#include "../Directory.h"
#include "LRUNode.h"
#include <stdint.h>
#include <cstring>
#include <set>
#include <string>

// An implementation of an extendable hashing directory employing an LRU eviction policy
class LRUBuffer: public Directory<LRUBufferEntry> {
public:
    LRUBuffer(int minSize, int maxSize, double min_load_factor = 0.25, double max_load_factor = 0.8);
    bool put(std::string file_and_page, uint8_t *data, int size) override;
    bool get(std::string file_and_page, std::uint8_t page_out_buf[PAGE_SIZE]) override;
    void evict() override;

private:
    LRUNode *head;
    LRUNode *tail;

    // make the entry corresponding to this node the most recent accessed entry
    void move_to_head(LRUNode *node);
};

#endif //KV_STORE_LRUBUFFER_H
