#ifndef KV_STORE_CLOCKBUFFER_H
#define KV_STORE_CLOCKBUFFER_H
#include "../Directory.h"
#include "ClockBufferEntry.h"
#include "../BufferPoolEntry.h"

#include "vector"

// An implementation of an extendable hashing directory employing a clock eviction policy
class ClockBuffer: public Directory<ClockBufferEntry> {
public:
    ClockBuffer(int minSize, int maxSize);
    void put(int page_num, std::uint8_t page[PAGE_SIZE]) override;
    int get(int page_num, std::uint8_t page_out_buf[PAGE_SIZE]) override;
    void evict() override;

private:
    // Points to the next eviction candidate
    ClockBufferEntry *clock_pointer;
    // Tracks the current bucket index containing the entry pointed to by the clock pointer
    int clock_entry_index;

    void increment_clock();
};


#endif //KV_STORE_CLOCKBUFFER_H
