#ifndef KV_STORE_CLOCKBUFFER_H
#define KV_STORE_CLOCKBUFFER_H
#include "../Directory.h"
#include "ClockBufferEntry.h"
#include "../BufferPoolEntry.h"
#include <stdint.h>
#include "vector"
#include <string>

// An implementation of an extendable hashing directory employing a clock eviction policy
class ClockBuffer: public Directory<ClockBufferEntry> {
public:
    ClockBuffer(int minSize, int maxSize, double min_load_factor = 0.25, double max_load_factor = 0.8);
    bool put(std::string file_and_page, std::uint8_t page[PAGE_SIZE]) override;
    bool get(std::string file_and_page, std::uint8_t page_out_buf[PAGE_SIZE]) override;
    void evict() override;

private:
    // Points to the next eviction candidate
    ClockBufferEntry *clock_pointer;
    // Tracks the current bucket index containing the entry pointed to by the clock pointer
    int clock_entry_index;

    void increment_clock();
};


#endif //KV_STORE_CLOCKBUFFER_H
