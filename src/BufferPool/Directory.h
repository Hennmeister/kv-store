#ifndef KV_STORE_DIRECTORY_H
#define KV_STORE_DIRECTORY_H

#include "vector"
#include "BufferPool.h"
#include <iostream>
#include "math.h"
#include "../util/MurmurHash3/MurmurHash3.h"


template <typename T> class Directory: BufferPool {
public:

    Directory(int min_size, int max_size);
    virtual void test() = 0;

    void set_size(int new_size) {
        if (new_size < 1) {
            // TODO: return err status
            return;
        }
        int new_num_bits = ceil(log2(min_size / (sizeof(T) * pow(10, 6) )));
        if (new_size < min_size) {
            min_size = new_size;
        }
        if (new_size > max_size) {
            max_size = new_size;
        }

        if (new_num_bits > num_bits) {
            grow(new_num_bits);
        } else if (new_num_bits < num_bits) {
            shrink(new_num_bits);
        }
    };

protected:
    int min_size;
    int max_size;
    int num_bits;
    int num_pages_in_buffer;
    std::vector<T *> entries;

    int hash_to_bucket_index(int page_num) {
        ::uint32_t bucket_num;
        MurmurHash3_x86_32(&page_num, sizeof(page_num), 1, &bucket_num);
        return bucket_num & ((1<<num_bits)-1);
    };

    void grow(int new_num_bits) {
        int size_diff = pow(2, new_num_bits) - pow(2, num_bits);

        // increase the number of entries (using NULL pointers so as not to allocate space yet)
        for (int i = 0; i < size_diff; i++) {
            entries.emplace_back(nullptr);
        }

        // iterate over all original buckets, and for all with overflowing chains rehash all elements
        for (int i = 0; i < pow(2, num_bits); i++) {
            T *curr_entry = entries[i];
            if (curr_entry != nullptr && curr_entry->next_entry != nullptr) {// entries exists and are overflowing
                curr_entry = curr_entry->next_entry;
                while (curr_entry != nullptr) {
                    auto temp = curr_entry->next_entry;
                    curr_entry->prev_entry->next_entry = curr_entry->next_entry;
                    curr_entry->next_entry->prev_entry = curr_entry->prev_entry;
                    curr_entry->next_entry = nullptr;
                    curr_entry->prev_entry = nullptr;

                    insert(curr_entry);

                    curr_entry = temp;
                }
            }
        }

        num_bits = new_num_bits;
    };

    void shrink(int new_num_bits) {
        int size_diff = pow(2, num_bits) - pow(2, new_num_bits);
        num_bits = new_num_bits;
        // iterate over all out-of-bounds entries, rehash them, and remove the entries
        T *curr_entry;
        T *next;
        for (int i = 0; i < size_diff; i++) {
            curr_entry = entries.back();
            while (curr_entry != nullptr) {
                next = curr_entry->next_entry;
                curr_entry->prev_entry = curr_entry->next_entry = nullptr;
                insert(curr_entry);
                curr_entry = next;
            }

            entries.pop_back();
        }
    };

    // internal helpers for page management
    void insert(T *entry);

    void delete_entry(T *entry);
};

template<typename T>
void Directory<T>::delete_entry(T *entry) {
    if (entry->prev_entry != nullptr) {
        entry->prev_entry->next_entry = entry->next_entry;
    }
    if (entry->next_entry != nullptr) {
        entry->next_entry->prev_entry = entry->prev_entry;
    }
    int bin = hash_to_bucket_index(entry->page_num);
    if (entry == entries[bin]) {
        entries[bin] = nullptr;
    }
    delete entry;
}

template<typename T>
void Directory<T>::insert(T *entry) {
    entry->prev_entry = nullptr;
    entry->next_entry = nullptr;

    ::uint32_t new_bucket_num = hash_to_bucket_index(entry->page_num);
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

    this->num_bits = ceil(log2((min_size * pow(10, 6)) / (sizeof(T))));

    for (int i = 0; i < (1<<num_bits); i++) {
        entries.emplace_back(nullptr);
    }
}


#endif //KV_STORE_DIRECTORY_H
