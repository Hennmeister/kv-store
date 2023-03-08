#ifndef KV_STORE_EVICTIONSTRATEGY_H
#define KV_STORE_EVICTIONSTRATEGY_H

#include <string>
#include "BufferPoolEntry.h"

class EvictionStrategy {
public:
    virtual ~EvictionStrategy() = default;
    virtual void evict() const = 0;
    virtual void insert_page(BufferPoolEntry page) const = 0;
};

#endif //KV_STORE_EVICTIONSTRATEGY_H