#ifndef KV_STORE_DIRECTORY_H
#define KV_STORE_DIRECTORY_H

#include "vector"
#include "../Base/BufferPool.h"
#include <iostream>
#include <cmath>
#include <set>
#include <map>
#include <string>
#include "../MurmurHash3.h"
using namespace std;

// An abstract class of a buffer pool using extendable hashing
template <typename T> class Directory: public BufferPool {
public:
    Directory(int min_size, int max_size, double min_load_factor, double max_load_factor);
    // Allow the user to increase or decrease the maximum hash table size
    void set_max_size(int new_size) override;

protected:
    // the load factor at which we double the directory
    double max_load_factor;
    // the minimum load factor we reach by shrinking directory after evicting entries to reach new max size
    double min_load_factor;
    int min_size;
    int max_size;
    // The current size of the directory is 2^(num_bits). Used to find the bucket number of a entry_data.
    int num_bits;
    int num_pages_in_buffer;
    int num_data_in_buffer;
    //
    std::vector<T *> entries;
    // Entry number to entry number pointing at this bucket, if one exists
    std::map<int, vector<int>> bucket_refs;
    // Holds a bucket number if it is referring to another bucket
    std::map<int, int> is_ref;

    virtual void evict() = 0;
    // Calculate the bucket number of a given entry_data
    int hash_to_bucket_index(std::string file_and_page);
    // Double the size of the directory
    void grow(int new_num_bits);
    // Shrink the directory itself - note that this does not evict entries, only the directory size
    void shrink();
    /// Internal helpers for entry_data management ///
    // Insert an entry into its correct bucket
    void insert(T *entry);
    // Delete a reference of an entry relative to its adjacent entries
    void delete_entry(T *entry);
};



#endif //KV_STORE_DIRECTORY_H
