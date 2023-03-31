#ifndef KV_STORE_DIRECTORY_H
#define KV_STORE_DIRECTORY_H

#include "vector"
#include "BufferPool.h"
#include <iostream>
#include "math.h"
#include <set>
#include <map>
#include "../util/MurmurHash3/MurmurHash3.h"
using namespace std;


template <typename T> class Directory: BufferPool {
public:
    Directory(int min_size, int max_size);
    void set_max_size(int new_size) override;

protected:
    double min_size;
    double max_size;
    int num_bits;
    int num_pages_in_buffer;
    std::vector<T *> entries;
    // entry number to entry number pointing at this bucket, if one exists
    std::map<int, vector<int>> bucket_refs;
    // holds a bucket number if it is referring to another bucket
    set<int> is_ref;

    virtual void evict() = 0;
    int hash_to_bucket_index(int page_num);
    void grow(int new_num_bits);
    // shrink the directory itself
    void shrink();
    // internal helpers for page management
    void insert(T *entry);
    void delete_entry(T *entry);
};



#endif //KV_STORE_DIRECTORY_H
