#ifndef KV_STORE_LRUBUFFER_H
#define KV_STORE_LRUBUFFER_H

#include "LRUBufferEntry.h"
#include "../Directory.h"
#include "LRUNode.h"
#include <set>

class LRUBuffer: public Directory<LRUBufferEntry> {
public:
    LRUBuffer(int minSize, int maxSize);
    void put(int page_num, std::uint8_t page[PAGE_SIZE]) override;
    int get(int page_num, std::uint8_t page_out_buf[PAGE_SIZE]) override;
    void evict() override;

    int count_pages() {
        int num = 0;
        set<int> seen_pages;
        for (int i = 0; i < entries.size(); i++) {
            LRUBufferEntry *curr = entries[i];
            while (curr != nullptr) {
                if (curr->page_num == 1) {
                    cout << "1 located at: " << i << endl;
                }
                seen_pages.insert(curr->page_num);
                curr = curr->next_entry;
            }
        }
        cout << "num pages, num_pages_in_buffer: " << seen_pages.size() << " " << num_pages_in_buffer << endl;
    };


private:
    LRUNode *head;
    LRUNode *tail;
};

#endif //KV_STORE_LRUBUFFER_H
