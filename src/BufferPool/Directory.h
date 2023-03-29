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

    void set_max_size(int new_size) override {
        if (new_size < min_size) {
            return; // TODO: add error here
        }
        max_size = new_size;
        while (num_pages_in_buffer * sizeof(T) > max_size * MB) {
            evict();
        }

        // shrink the directory until we are using at least 25% of our directory entries
//        int new_num_bits = num_bits;
//        while ((num_pages_in_buffer / (1 << new_num_bits)) < 0.25) {
//            new_num_bits--;
//        }
//
//        int size_diff = pow(2, num_bits) - pow(2, new_num_bits);
//        cout << num_bits << " , " << new_num_bits << endl;
//        num_bits = new_num_bits;
//        cout << "size_diff: " << size_diff << endl;
//        shrink(size_diff);
    };

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




    int hash_to_bucket_index(int page_num) {
        ::uint32_t bucket_num;
        MurmurHash3_x86_32(&page_num, sizeof(page_num), 1, &bucket_num);
        return bucket_num & ((1<<num_bits)-1);
    };

    void grow(int new_num_bits) {
        int size_diff = pow(2, new_num_bits) - pow(2, num_bits);
        // increase the number of entries (using NULL pointers so as not to allocate space yet)
        for (int i = 0; i < size_diff; i++) {
            entries.emplace_back(entries[i]);
            if (entries[i] == nullptr) continue;
            auto it = bucket_refs.find(i);
            if (it != bucket_refs.end()) {
                it->second.push_back(i + size_diff);
            } else {
                vector<int> v{i + size_diff};
                bucket_refs.insert(pair<int, vector<int>>(i, v));
            }
        }
        num_bits = new_num_bits;


        // iterate over all original buckets, and for all with overflowing chains rehash all elements
        for (int i = 0; i < size_diff; i++) {
            T *curr_entry = entries[i];

            if (curr_entry != nullptr && curr_entry->next_entry != nullptr) { // entries exists and are overflowing
                // rehash all entries in this bucket
                // set all entries that point to this bucket to nullptr
                // (since no elements in the original bucket belongs in this bucket after rehashing)
                entries[i] = nullptr;
                vector<int> queue{i};
                while (!queue.empty()) {
                    auto it = bucket_refs.find(queue.back());
                    queue.pop_back();
                    if (it == bucket_refs.end()) continue;
                    for (int erase_bucket_num : it->second) {
                        queue.push_back(erase_bucket_num);
                        entries[erase_bucket_num] = nullptr;
                    }
                    bucket_refs.erase(it);
                    // continues down the chain of references, since these all point to the original bucket
                }

                while (curr_entry != nullptr) {
                    auto temp = curr_entry->next_entry;
                    curr_entry->next_entry = nullptr;
                    curr_entry->prev_entry = nullptr;
                    insert(curr_entry);
                    curr_entry = temp;
                }
            }
        }
    };

    // shrink the directory itself
    void shrink(int size_diff) {
        // iterate over all out-of-bounds entries, rehash them, and move the entries
        T *curr_entry;
        T *next;
        while (entries.size() > pow(2, num_bits)) {
            // TODO: only reinsert / rehash these entries if this is not a reference to an earlier bucket
            curr_entry = entries.back();
            while (curr_entry != nullptr) {
                cout << curr_entry->page_num << endl;
                next = curr_entry->next_entry;
                curr_entry->prev_entry = curr_entry->next_entry = nullptr;
                insert(curr_entry);
                curr_entry = next;
            }
            entries.pop_back();
        }
        std::map<int, vector<int>>::iterator it;
        int cutoff = entries.size();
        for (it = bucket_refs.begin(); it != bucket_refs.end(); it++) {
            if (it->first >= cutoff) {
                it = bucket_refs.erase(it);
            } else {
                vector<int>::iterator vector_it;
                for (vector_it = it->second.begin(); vector_it !=  it->second.end(); vector_it++) {
                    if (*vector_it >= cutoff) {
                        vector_it = it->second.erase(vector_it);
                    }
                }
            }

        }
    };

    // internal helpers for page management
    void insert(T *entry);

    void delete_entry(T *entry);
};

template<typename T>
void Directory<T>::delete_entry(T *entry) {
    // TODO: have references to all other buckets pointing to this
    for (int i = 0; i < entries.size(); i++) {
        if (entries[i] == entry) {
            entries[i] = entry->next_entry;
        }
    }

    if (entry->prev_entry != nullptr) {
        entry->prev_entry->next_entry = entry->next_entry;
    }
    if (entry->next_entry != nullptr) {
        entry->next_entry->prev_entry = entry->prev_entry;
    }

    delete entry;
}

template<typename T>
void Directory<T>::insert(T *entry) {
    entry->prev_entry = nullptr;
    entry->next_entry = nullptr;

    uint32_t new_bucket_num = hash_to_bucket_index(entry->page_num);
    T *curr = entries[new_bucket_num];
    if (curr == nullptr) {
        entries[new_bucket_num] = entry;
        return;
    }

    while (curr->next_entry != nullptr) {
        curr = curr->next_entry;
    }
    curr->next_entry = entry;
    entry->prev_entry = curr;
}

template<typename T>
Directory<T>::Directory(int min_size, int max_size) {
    this->min_size = min_size;
    this->max_size = max_size;
    this->num_pages_in_buffer = 0;

    this->num_bits = ceil(log2((min_size * MB) / sizeof(T)));

    for (int i = 0; i < (1<<num_bits); i++) {
        entries.emplace_back(nullptr);
    }
}


#endif //KV_STORE_DIRECTORY_H
