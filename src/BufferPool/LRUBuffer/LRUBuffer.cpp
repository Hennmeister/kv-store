#include "LRUBuffer.h"

LRUBuffer::LRUBuffer(int minSize, int maxSize) : Directory(minSize, maxSize) {
    head = nullptr;
    tail = nullptr;
}

void LRUBuffer::put(int page_num, std::uint8_t *page) {
    // check if at capacity and if so evict
    if ((num_pages_in_buffer+1) * sizeof(LRUBufferEntry) >= max_size * MB) {
        evict();
    }

    // check if we should grow directory
    if ((num_pages_in_buffer / (1<<num_bits)) > 0.75) {
        grow(num_bits + 1);
    }

    // insert page
    LRUBufferEntry *entry = new LRUBufferEntry();
    entry->page_num = page_num;
    ::memcpy(entry->page, page, PAGE_SIZE);
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
}

int LRUBuffer::get(int page_num, std::uint8_t *page_out_buf) {
    int bucket_num = hash_to_bucket_index(page_num);
    LRUBufferEntry *curr_entry = entries[bucket_num];
    while (curr_entry != nullptr && curr_entry->page_num != page_num) {
        curr_entry = curr_entry->next_entry;
    }
    if (curr_entry == nullptr) {
        return -1;
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
    ::memcpy(page_out_buf, curr_entry->page, PAGE_SIZE);
    return 0;
}

void LRUBuffer::evict() {
    LRUNode *new_tail = tail->prev;
    delete_entry(tail->bufferEntry);
    num_pages_in_buffer--;
    delete tail;
    tail = new_tail;
    tail->next = nullptr;
}
