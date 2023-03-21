#ifndef KV_STORE_LRUBUFFERENTRY_H
#define KV_STORE_LRUBUFFERENTRY_H

#include "../BufferPoolEntry.h"
#include "LRUNode.h"


struct LRUBufferEntry: BufferPoolEntry<LRUBufferEntry> {
    LRUNode *node = nullptr;
};


#endif //KV_STORE_LRUBUFFERENTRY_H
