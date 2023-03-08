#ifndef KV_STORE_BUFFERPOOLENTRY_H
#define KV_STORE_BUFFERPOOLENTRY_H

// TODO: make this a struct?
class BufferPoolEntry {
public:
    char dirty; // 1 indicates page is dirty, 0 otherwise
    void* page; // 4KB page
    BufferPoolEntry *next_entry;
    BufferPoolEntry *prev_entry; // for eviction

};


#endif //KV_STORE_BUFFERPOOLENTRY_H
