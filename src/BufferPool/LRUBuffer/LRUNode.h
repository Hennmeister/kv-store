#ifndef KV_STORE_LRUNODE_H
#define KV_STORE_LRUNODE_H

#include "LRUBufferEntry.h"

struct LRUNode {
    LRUBufferEntry *bufferEntry;
    LRUNode *next = nullptr;
    LRUNode *prev = nullptr;
};

#endif //KV_STORE_LRUNODE_H
