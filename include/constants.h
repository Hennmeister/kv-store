
#ifndef KV_STORE_CONSTANTS_H
#define KV_STORE_CONSTANTS_H

#define PAGE_SIZE 4096

// Entry in a page: sizeof(key) + sizeof(val) = 8B (2 ints)
#define ENTRY_SIZE 8

#define PAGE_NUM_ENTRIES (PAGE_SIZE/ENTRY_SIZE) // 512


#endif //KV_STORE_CONSTANTS_H
