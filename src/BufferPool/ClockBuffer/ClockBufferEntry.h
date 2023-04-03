#ifndef KV_STORE_CLOCKBUFFERENTRY_H
#define KV_STORE_CLOCKBUFFERENTRY_H

#include "../BufferPoolEntry.h"


struct ClockBufferEntry: BufferPoolEntry<ClockBufferEntry> {
    unsigned char used_bit = 0;
};

#endif //KV_STORE_CLOCKBUFFERENTRY_H
