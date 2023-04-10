#include "../../../include/BufferPool/LRUBuffer/LRUBuffer.h"
#include <stdint.h>


using namespace std;

// Note that min_load_factor and max_load_factor are optional
LRUBuffer::LRUBuffer(int minSize, int maxSize, double min_load_factor, double max_load_factor) : Directory(minSize, maxSize, min_load_factor, max_load_factor) {
    head = nullptr;
    tail = nullptr;
}

// Inserts a new entry_data into the buffer, evicting the LRU entry_data if necessary.
// If the directory is at capacity, evicts the LRU entry_data.
// If the directory is more than 75% full, grows the directory to increase capacity
bool LRUBuffer::put(std::string file_and_page, uint8_t *data, int size) {
    // evict if the buffer pool if near capacity
    if(max_size == 0){
        return true;
    }

    if (num_data_in_buffer + size > (max_size * MB) * max_load_factor) {
        evict();
    }

    // grow the directory if we are at a high enough load factor
    if (((double) num_pages_in_buffer / (1<<num_bits)) > max_load_factor) {
        grow(num_bits + 1);
    }

    // do a lookup for this entry and replace its data if it exists
    int bucket_num = hash_to_bucket_index(file_and_page);
    LRUBufferEntry *curr_entry = entries[bucket_num];
    while (curr_entry != nullptr && curr_entry->file_and_page != file_and_page) {
        curr_entry = curr_entry->next_entry;
    }
    if (curr_entry != nullptr) {
        move_to_head(curr_entry->node);
        // note that size must be equal to entry data size
        memcpy(curr_entry->entry_data, data, size);
        return true;
    }

    // insert the new entry_data
    LRUBufferEntry *entry = new LRUBufferEntry();
    entry->file_and_page = file_and_page;

    uint8_t *entry_data = new uint8_t[size];
    memcpy(entry_data, data, size);
    entry->page_size = size;
    entry->entry_data = entry_data;
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
    num_data_in_buffer += size;
    return true;
}

// Retrieve the entry_data with a entry_data number of page_num, filling the page_out_buf, or return -1 otherwise
// Make retrieved entry_data the most recently used entry
// TODO: return not found error status
bool LRUBuffer::get(string file_and_page, uint8_t *page_out_buf) {
    if(num_data_in_buffer == 0) return false;

    int bucket_num = hash_to_bucket_index(file_and_page);
    LRUBufferEntry *curr_entry = entries[bucket_num];
    while (curr_entry != nullptr && curr_entry->file_and_page != file_and_page) {
        curr_entry = curr_entry->next_entry;
    }
    if (curr_entry == nullptr) {
        return false;
    }

    move_to_head(curr_entry->node);

    // TODO: verify that we should be copying here, instead of using pointer pointer and changing address
    memcpy(page_out_buf, curr_entry->entry_data, curr_entry->page_size);
    return true;
}

// Delete the entry least recently used
void LRUBuffer::evict() {
    // remove all the references to evicted buckets
    auto next_page_in_bucket = tail->bufferEntry->next_entry;
    auto it = bucket_refs.find(hash_to_bucket_index(tail->bufferEntry->file_and_page));
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

    LRUNode *new_tail = tail->prev;
    num_data_in_buffer -= tail->bufferEntry->page_size;
    delete_entry(tail->bufferEntry);
    num_pages_in_buffer--;
    delete tail;
    tail = new_tail;
    tail->next = nullptr;
}
// move the corresponding LRU node to head of LRU linked list tracking recency
void LRUBuffer::move_to_head(LRUNode *node) {
    if (tail == node) tail = node->prev;
    if (node->prev != nullptr) node->prev->next = node->next;
    if (node->next != nullptr) node->next->prev = node->prev;
    node->next = head;
    node->prev = nullptr;
    head->prev = node;
    head = node;
}
