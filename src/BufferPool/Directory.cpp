#include "Directory.h"
#include "../util/MurmurHash3/MurmurHash3.h"
#include <iostream>

void Directory::insert_page(uint32_t page_num, ::uint8_t *page) {

}

// TODO: create proper statuses/error messages to return
int Directory::get_page(uint32_t page_num, uint8_t *page_out_buf) {
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

void Directory::grow() {
    if (1<<(num_bits+1) > max_size) {
        // TODO: error here
        cout << "err: attempting to grow past max buffer size" << endl;
        return;
    }

    num_bits++;
    // double the number of entries (using NULL pointers so as not to allocate space yet)

    // iterate over all original buckets, and for all with overflowing chains rehash all elements
}

std::uint32_t Directory::hash_to_bucket_index(std::uint32_t page_num) {
    ::uint32_t bucket_num;
    MurmurHash3_x86_32(&page_num, sizeof(page_num), 1, &bucket_num);
    return bucket_num^(1<<(num_bits-1));
}

Directory::Directory(int min_size, int max_size) {
    this->min_size = min_size;
    this->max_size = max_size;
    // TODO: this->num_bits =
}
