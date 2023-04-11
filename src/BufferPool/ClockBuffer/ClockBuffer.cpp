#include "../../../include/BufferPool/ClockBuffer/ClockBuffer.h"
#include "../../../include/BufferPool/ClockBuffer/ClockBufferEntry.h"
#include "map"
#include <cstring>
#include <stdint.h>

using namespace std;

// Note that min_load_factor and max_load_factor are optional
ClockBuffer::ClockBuffer(int minSize, int maxSize, double min_load_factor, double max_load_factor) : Directory(minSize, maxSize, min_load_factor, max_load_factor) {
    clock_pointer = nullptr;
    clock_entry_index = 0;
}

// Inserts a new entry_data into the buffer, evicting the next entry pointed to by the clock
// pointer with a used used_bit of 0
// If the directory is at capacity, evicts the LRU entry_data.
// If the directory is more than 75% full, grows the directory to increase capacity
bool ClockBuffer::put(std::string file_and_page, uint8_t *data, int size) {
    if (max_size == 0) return true;

    // evict if the buffer pool is at capacity
    if (num_data_in_buffer + size > (max_size * MB) * max_load_factor) {
       evict();
    }

    // grow the directory if we are at a load factor of at least 75%
    if (((double) num_pages_in_buffer / (1<<num_bits)) > max_load_factor) {
        grow(num_bits + 1);
    }


    // do a lookup for this entry and replace its data if it exists
    int bucket_num = hash_to_bucket_index(file_and_page);
    ClockBufferEntry *curr_entry = entries[bucket_num];
    while (curr_entry != nullptr && curr_entry->file_and_page != file_and_page) {
        curr_entry = curr_entry->next_entry;
    }
    if (curr_entry != nullptr) {
        curr_entry->used_bit = 1;
        // note that size must be equal to entry data size
        memcpy(curr_entry->entry_data, data, size);
        return true;
    }

    // insert the new page
    ClockBufferEntry *entry = new ClockBufferEntry();
    entry->file_and_page = file_and_page;
    uint8_t *entry_data = new uint8_t[size];
    memcpy(entry_data, data, size);
    entry->page_size = size;
    entry->entry_data = entry_data;
    entry->prev_entry = entry->next_entry = nullptr;

    insert(entry);
    num_pages_in_buffer++;
    num_data_in_buffer += size;
    if (num_pages_in_buffer == 1) {
        clock_pointer = entry;
        clock_entry_index = hash_to_bucket_index(entry->file_and_page);
    }
    return true;
}

// Retrieve the entry_data with a entry_data number of page_num, filling the page_out_buf, or return -1 otherwise
// Update used used_bit of requested entry
// TODO: return not found error status
bool ClockBuffer::get(string file_and_page, uint8_t *page_out_buf) {
    if (num_data_in_buffer == 0) return false;

    int bucket_num = hash_to_bucket_index(file_and_page);
    ClockBufferEntry *curr_entry = entries[bucket_num];
    while (curr_entry != nullptr && curr_entry->file_and_page != file_and_page) {
        curr_entry = curr_entry->next_entry;
    }
    if (curr_entry == nullptr) {
        return false;
    }
    // set this entries clock used_bit since it was referenced
    curr_entry->used_bit = 1;
    // TODO: verify that we should be copying here, instead of using pointer pointer and changing address
    memcpy(page_out_buf, curr_entry->entry_data, curr_entry->page_size);
    return true;
}

// finds the next entry for the clock
void ClockBuffer::increment_clock() {
    // set the used used_bit of the entry we are moving off of to 0
    clock_pointer->used_bit = 0;
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

// Delete the next entry that has not been used since the last time the clock pointer referenced it
void ClockBuffer::evict() {
    while (clock_pointer->used_bit == 1) {
        increment_clock();
    }
    // remove all the references to evicted buckets
    auto next_page_in_bucket = clock_pointer->next_entry;
    auto it = bucket_refs.find(hash_to_bucket_index(clock_pointer->file_and_page));
    // the bucket holding the entry_data to be evicted has at least one other bucket pointing to it
    if (it != bucket_refs.end()) {
        for (auto vector_it = it->second.begin(); vector_it != it->second.end(); vector_it++) {
            entries[*vector_it] = next_page_in_bucket; // could be nullptr
            // if we are emptying this bucket by deleting the last entry_data in the bucket,
            // unmark buckets pointing to this bucket as references
            if (next_page_in_bucket == nullptr) {
                is_ref.erase(*vector_it);
            }
        }
        // if we are deleting the last entry in this bucket, nothing will be referencing this bucket anymore
        if (next_page_in_bucket == nullptr) {
            bucket_refs.erase(it);
        }
    }

    ClockBufferEntry *entry_to_delete = clock_pointer;
    num_pages_in_buffer--;
    num_data_in_buffer -= entry_to_delete->page_size;
    increment_clock();
    delete_entry(entry_to_delete);
}

