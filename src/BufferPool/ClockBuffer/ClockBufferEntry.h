#ifndef KV_STORE_CLOCKBUFFERENTRY_H
#define KV_STORE_CLOCKBUFFERENTRY_H

#include "../BufferPoolEntry.h"


struct ClockBufferEntry: BufferPoolEntry<ClockBufferEntry> {
    short used_bit = 0;
};

#endif //KV_STORE_CLOCKBUFFERENTRY_H
