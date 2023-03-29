#include "ClockBuffer.h"
#include "ClockBufferEntry.h"
#include "map"

ClockBuffer::ClockBuffer(int minSize, int maxSize) : Directory(minSize, maxSize) {
    clock_pointer = nullptr;
    clock_entry_index = 0;
}

void ClockBuffer::put(int page_num, std::uint8_t *page) {
    // check if at capacity and if so evict
    if ((num_pages_in_buffer+1) * sizeof(ClockBufferEntry) >= max_size * MB) {
       evict();
    }

    // check if we should grow directory
    if ((num_pages_in_buffer / (1<<num_bits)) > 0.75) {
        grow(num_bits + 1);
    }

    // insert page
    ClockBufferEntry *entry = new ClockBufferEntry();
    entry->page_num = page_num;
    ::memcpy(entry->page, page, PAGE_SIZE);
    entry->prev_entry = entry->next_entry = nullptr;

    insert(entry);
    num_pages_in_buffer++;
    if (num_pages_in_buffer == 1) {
        clock_pointer = entry;
        clock_entry_index = hash_to_bucket_index(entry->page_num);
    }
}

int ClockBuffer::get(int page_num, std::uint8_t *page_out_buf) {
    int bucket_num = hash_to_bucket_index(page_num);
    ClockBufferEntry *curr_entry = entries[bucket_num];
    while (curr_entry != nullptr && curr_entry->page_num != page_num) {
        curr_entry = curr_entry->next_entry;
    }
    if (curr_entry == nullptr) {
        return -1;
    }
    // set this entries clock bit since it was referenced
    curr_entry->bit = 1;
    // TODO: verify that we should be copying here, instead of using pointer pointer and changing address
    ::memcpy(page_out_buf, curr_entry->page, PAGE_SIZE);
    return 0;
}

// finds the next entry for the clock
void ClockBuffer::increment_clock() {
    // set the bit of the entry we are moving off of to 0
    clock_pointer->bit = 0;
    if (clock_pointer->next_entry != nullptr) {
        clock_pointer = clock_pointer->next_entry;
        return;
    }
    else {
        clock_pointer = nullptr;
        while (clock_pointer == nullptr) {
            clock_entry_index = (clock_entry_index + 1) % (int) entries.size();
            clock_pointer = entries[clock_entry_index];
        }
    }
}

void ClockBuffer::evict() {
    // reached page to evict
    while (clock_pointer->bit == 1) {
        increment_clock();
    }
    ClockBufferEntry *entry_to_delete = clock_pointer;
    num_pages_in_buffer--;
    increment_clock();
    // deleting this entry, but setting bit to 1 so that clock pointer increments
    delete_entry(entry_to_delete);
}

