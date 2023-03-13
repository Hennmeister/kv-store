#include "Directory.h"
#include "../util/MurmurHash3/MurmurHash3.h"
#include "math.h"

#include <bitset>

// Precondition: page is not in Buffer pool already
void Directory::insert_page(uint32_t page_num, ::uint8_t page[PAGE_SIZE]) {
    if (num_pages_in_buffer * sizeof(BufferPoolEntry) >= max_size) {
        eviction_strategy->evict();
    }

    // Currently increasing buffer size when load factor reaches 75% and under max size
    if (((float) num_pages_in_buffer / (float) (1<<  num_bits)) > 0.75) {
        grow(num_bits + 1);
    }

    auto *entry = new BufferPoolEntry();
    entry->page_num = page_num;
    ::memcpy(entry->page, page, PAGE_SIZE);
    entry->prev_entry = entry->next_entry = nullptr;
    entry->dirty = 0;

    insert(entry);
}

// TODO: create proper statuses/error messages to return
int Directory::get_page(uint32_t page_num, uint8_t page_out_buf[PAGE_SIZE]) {
    ::uint32_t bucket_num = hash_to_bucket_index(page_num);
    BufferPoolEntry *curr_entry = entries[bucket_num];
    while (curr_entry != nullptr && curr_entry->page_num != page_num) {
        curr_entry = curr_entry->next_entry;
    }
    if (curr_entry == nullptr) {
        return -1;
    }
    // TODO: verify that we should be copying here, instead of using pointer pointer and changing address
    ::memcpy(page_out_buf, curr_entry->page, PAGE_SIZE);
}

// Precondition: new_num_bits implies a size smaller than max_size and is larger than the current num_bits
void Directory::grow(int new_num_bits) {
    int size_diff = pow(2, new_num_bits) - pow(2, num_bits);

    // increase the number of entries (using NULL pointers so as not to allocate space yet)
    for (int i = 0; i < size_diff; i++) {
        entries.emplace_back(nullptr);
    }

    // iterate over all original buckets, and for all with overflowing chains rehash all elements
    for (int i = 0; i < pow(2, num_bits); i++) {
        BufferPoolEntry *curr_entry = entries[i];
        if (curr_entry != nullptr && curr_entry->next_entry != nullptr) {// entries exists and are overflowing
            curr_entry = curr_entry->next_entry;
            while (curr_entry != nullptr) {
                auto temp = curr_entry->next_entry;
                curr_entry->prev_entry->next_entry = curr_entry->next_entry;
                curr_entry->next_entry->prev_entry = curr_entry->prev_entry;
                curr_entry->next_entry = nullptr;
                curr_entry->prev_entry = nullptr;

                insert(curr_entry);

                curr_entry = temp;
            }
        }
    }

    num_bits = new_num_bits;
}

void Directory::shrink(int new_num_bits) {
    int size_diff = pow(2, num_bits) - pow(2, new_num_bits);
    num_bits = new_num_bits;
    // iterate over all out-of-bounds entries, rehash them, and remove the entries
    BufferPoolEntry *curr_entry;
    BufferPoolEntry *next;
    for (int i = 0; i < size_diff; i++) {
        curr_entry = entries.back();
        while (curr_entry != nullptr) {
            next = curr_entry->next_entry;
            curr_entry->prev_entry = curr_entry->next_entry = nullptr;
            insert(curr_entry);
            curr_entry = next;
        }

        entries.pop_back();
    }
}

// Precondition: new size is in MB
void Directory::set_size(int new_size) {
    if (new_size < 1) {
        // TODO: return err status
        return;
    }
    int new_num_bits = ceil(log2(min_size / (sizeof(BufferPoolEntry) * pow(10, 6) )));
    if (new_size < min_size) {
        min_size = new_size;
    }
    if (new_size > max_size) {
        max_size = new_size;
    }

    if (new_num_bits > num_bits) {
        grow(new_num_bits);
    } else if (new_num_bits < num_bits) {
        shrink(new_num_bits);
    }

}


std::uint32_t Directory::hash_to_bucket_index(std::uint32_t page_num) const {
    ::uint32_t bucket_num;
    MurmurHash3_x86_32(&page_num, sizeof(page_num), 1, &bucket_num);
    return bucket_num & ((1<<num_bits)-1);
}

// assuming sizes are passed in as MB
Directory::Directory(int min_size, int max_size, EvictionStrategy *eviction_strategy) {
    this->min_size = min_size;
    this->max_size = max_size;
    this->eviction_strategy = eviction_strategy;
    this->num_pages_in_buffer = 0;

    this->num_bits = ceil(log2((min_size * pow(10, 6)) / (sizeof(BufferPoolEntry))));

    for (int i = 0; i < (1<<num_bits); i++) {
        entries.emplace_back(nullptr);
    }
}

// assumes this entry is not currently in buffer pool and
void Directory::insert(BufferPoolEntry *entry) {
    entry->prev_entry = nullptr;
    entry->next_entry = nullptr;

    ::uint32_t new_bucket_num = hash_to_bucket_index(entry->page_num);
    BufferPoolEntry *curr = entries[new_bucket_num];
    if (curr == nullptr) {
        entries[new_bucket_num] = entry;
        return;
    }

    while (curr->next_entry != nullptr) {
        curr = curr->next_entry;
    }
    curr->next_entry = entry;
    entry->prev_entry = curr;
}