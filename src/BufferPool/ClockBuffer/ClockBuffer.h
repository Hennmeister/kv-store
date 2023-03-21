#ifndef KV_STORE_CLOCKBUFFER_H
#define KV_STORE_CLOCKBUFFER_H
#include "../Directory.h"
#include "ClockBufferEntry.h"
#include "../BufferPoolEntry.h"

#include "vector"

class ClockBuffer: public Directory<ClockBufferEntry> {
public:
    ClockBuffer(int minSize, int maxSize);
    void put(int page_num, std::uint8_t page[PAGE_SIZE]) override;
    int get(int page_num, std::uint8_t page_out_buf[PAGE_SIZE]) override;

private:
    std::vector<std::pair<int, int>> bitmap;
    ClockBufferEntry *clock_pointer;
    int clock_entry_index;
};


#endif //KV_STORE_CLOCKBUFFER_H
