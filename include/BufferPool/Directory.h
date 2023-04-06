#ifndef KV_STORE_DIRECTORY_H
#define KV_STORE_DIRECTORY_H

#include "vector"
#include "../Base/BufferPool.h"
#include <iostream>
#include "math.h"
#include <set>
#include <map>
#include "../MurmurHash3.h"
using namespace std;

// An abstract class of a buffer pool using extendable hashing
template <typename T> class Directory: public BufferPool {
public:
    Directory(int min_size, int max_size);
    // Allow the user to increase or decrease the maximum hash table size
    void set_max_size(int new_size) override;

protected:
    // the load factor at which we double the directory
    const double max_load_factor = 0.8;
    // the minimum load factor we reach by shrinking directory after evicting entries to reach new max size
    const double min_load_factor = 0.25;
    int min_size;
    int max_size;
    // The current size of the directory is 2^(num_bits). Used to find the bucket number of a page.
    int num_bits;
    int num_pages_in_buffer;
    //
    std::vector<T *> entries;
    // Entry number to entry number pointing at this bucket, if one exists
    std::map<int, vector<int>> bucket_refs;
    // Holds a bucket number if it is referring to another bucket
    set<int> is_ref;

    virtual void evict() = 0;
    // Calculate the bucket number of a given page
    int hash_to_bucket_index(uint32_t page_num);
    // Double the size of the directory
    void grow(int new_num_bits);
    // Shrink the directory itself - note that this does not evict entries, only the directory size
    void shrink();
    /// Internal helpers for page management ///
    // Insert an entry into its correct bucket
    void insert(T *entry);
    // Delete a reference of an entry relative to its adjacent entries
    void delete_entry(T *entry);
};



#endif //KV_STORE_DIRECTORY_H
