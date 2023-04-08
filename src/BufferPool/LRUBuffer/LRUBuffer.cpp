#include "../../../include/BufferPool/LRUBuffer/LRUBuffer.h"
#include <stdint.h>


using namespace std;

// Note that min_load_factor and max_load_factor are optional
LRUBuffer::LRUBuffer(int minSize, int maxSize, double min_load_factor, double max_load_factor) : Directory(minSize, maxSize, min_load_factor, max_load_factor) {
    head = nullptr;
    tail = nullptr;
}

// Inserts a new page into the buffer, evicting the LRU page if necessary.
// If the directory is at capacity, evicts the LRU page.
// If the directory is more than 75% full, grows the directory to increase capacity
bool LRUBuffer::put(string file_and_page, uint8_t page[4096]) {
    // evict if the buffer pool if near capacity
    if ((double) (num_pages_in_buffer+1) * sizeof(LRUBufferEntry) > (max_size * MB) * max_load_factor) {
        evict();
    }

    // grow the directory if we are at a high enough load factor
    if (((double) num_pages_in_buffer / (1<<num_bits)) > max_load_factor) {
        grow(num_bits + 1);
    }

    // insert the new page
    LRUBufferEntry *entry = new LRUBufferEntry();
    entry->file_and_page = file_and_page;
    memcpy(entry->page, page, PAGE_SIZE);
    entry->prev_entry = entry->next_entry = nullptr;

    LRUNode *node = new LRUNode();
    entry->node = node;
    node->bufferEntry = entry;
    node->next = head;
    if (head != nullptr) head->prev = node;
    head = node;
    if (tail == nullptr) tail = head;

    insert(entry);
    num_pages_in_buffer++;
    return true;
}

// Retrieve the page with a page number of page_num, filling the page_out_buf, or return -1 otherwise
// Make retrieved page the most recently used entry
// TODO: return not found error status
bool LRUBuffer::get(string file_and_page, uint8_t page_out_buf[4096]) {
    int bucket_num = hash_to_bucket_index(file_and_page);
    LRUBufferEntry *curr_entry = entries[bucket_num];
    while (curr_entry != nullptr && curr_entry->file_and_page != file_and_page) {
        curr_entry = curr_entry->next_entry;
    }
    if (curr_entry == nullptr) {
        return false;
    }

    // move the corresponding LRU node to head of LRU linked list tracking recency
    LRUNode *node = curr_entry->node;
    if (tail == node) tail = node->prev;
    if (node->prev != nullptr) node->prev->next = node->next;
    if (node->next != nullptr) node->next->prev = node->prev;
    node->next = head;
    node->prev = nullptr;
    head->prev = node;
    head = node;

    // TODO: verify that we should be copying here, instead of using pointer pointer and changing address
    memcpy(page_out_buf, curr_entry->page, PAGE_SIZE);
    return true;
}

// Delete the entry least recently used
void LRUBuffer::evict() {
    // remove all the references to evicted buckets
    auto next_page_in_bucket = tail->bufferEntry->next_entry;
    auto it = bucket_refs.find(hash_to_bucket_index(tail->bufferEntry->file_and_page));
    // the bucket holding the page to be evicted has at least one other bucket pointing to it
    if (it != bucket_refs.end()) {
        for (auto vector_it = it->second.begin(); vector_it != it->second.end(); vector_it++) {
            entries[*vector_it] = next_page_in_bucket; // could be nullptr
            // if we are emptying this bucket by deleting the last page in the bucket,
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

    LRUNode *new_tail = tail->prev;
    delete_entry(tail->bufferEntry);
    num_pages_in_buffer--;
    delete tail;
    tail = new_tail;
    tail->next = nullptr;
}
