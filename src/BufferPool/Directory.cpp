#include "Directory.h"
#include "LRUBuffer/LRUBufferEntry.h"
#include "ClockBuffer/ClockBufferEntry.h"

template <typename T> Directory<T>::Directory(int min_size, int max_size){
    this->min_size = min_size;
    this->max_size = max_size;
    this->num_pages_in_buffer = 0;

    this->num_bits = ceil(log2((min_size * MB) / sizeof(T)));

    for (int i = 0; i < (1<<num_bits); i++) {
        entries.emplace_back(nullptr);
    }
}

// Allow the user to increase or decrease the maximum hash table size
template<typename T>
void Directory<T>::set_max_size(int new_size) {
    if (new_size < min_size) {
        return; // TODO: add error here
    }
    max_size = new_size;
    while (num_pages_in_buffer * sizeof(T) > (max_size * MB) * max_load_factor) {
        evict();
    }

    //  shrink the directory until we reach a load factor of a least 25%
    int new_num_bits = num_bits;
    while (((double) num_pages_in_buffer / (double) (1 << new_num_bits)) < min_load_factor) {
        new_num_bits--;
    }

    num_bits = new_num_bits;
    shrink();
};

// Hash from a page number to a bucket index, using the number of bits to map to a bucket within the table
template<typename T>
int Directory<T>::hash_to_bucket_index(uint32_t page_num) {
    uint32_t bucket_num;
    MurmurHash3_x86_32(&page_num, sizeof(page_num), 1, &bucket_num);
    return bucket_num & ((1<<num_bits)-1);
};

// Double the size of the hashtable, having new directory entries initially map to one initial entry
// rehashing elements in overflowing (chains of greater than 1) buckets
template<typename T>
void Directory<T>::grow(int new_num_bits) {
    // calculate the size difference between the old and new directory sizes
    int size_diff = pow(2, new_num_bits) - pow(2, num_bits);

    // increase the number of entries (using NULL pointers so as not to allocate space yet)
    for (int i = 0; i < size_diff; i++) {
        // Add new null pointer entries to the end of the vector
        entries.emplace_back(entries[i]);

        // if the old bucket at index i is non-null, then create a reference to it in the new section of the directory
        if (entries[i] == nullptr) continue;
        is_ref.insert(i + size_diff);

        // update the bucket references for the old bucket
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

        // if the current entry exists and its chain of entries is overflowing, rehash all entries in this bucket
        if (curr_entry != nullptr && curr_entry->next_entry != nullptr) {
            // set all entries that point to this bucket to nullptr since no elements in the original bucket belongs
            // in this bucket after rehashing
            entries[i] = nullptr;
            is_ref.erase(i);
            vector<int> queue{i};

            // erase all references to the current bucket and its overflown buckets
            while (!queue.empty()) {
                auto it = bucket_refs.find(queue.back());
                queue.pop_back();
                if (it == bucket_refs.end()) continue;
                for (int erase_bucket_num : it->second) {
                    queue.push_back(erase_bucket_num);
                    entries[erase_bucket_num] = nullptr;
                    is_ref.erase(erase_bucket_num);
                }
                bucket_refs.erase(it);
            }

            // reinsert all elements in the chain of the old bucket
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

// Shrink the size of the directory (not evicting entries) by
//  1) rehashing out of bound entries and
//  2) updating the references to evicted buckets
template<typename T>
void Directory<T>::shrink() {
    // iterate over all out-of-bounds entries, rehash them, and move the entries
    T *curr_entry;
    T *next;
    for (int i = entries.size() - 1; i >= pow(2, num_bits); i--) {
        curr_entry = entries.back();
        // only reinsert / rehash these entries if this is not a reference to an earlier bucket
        if (is_ref.find(i) == is_ref.end()) {
            while (curr_entry != nullptr) {
                next = curr_entry->next_entry;
                curr_entry->prev_entry = curr_entry->next_entry = nullptr;
                insert(curr_entry);
                curr_entry = next;
            }
        }
        is_ref.erase(i);

        entries.pop_back();
    }

    // remove all the references to evicted buckets
    std::map<int, vector<int>>::iterator it;
    int cutoff = entries.size();
    for (it = bucket_refs.begin(); it != bucket_refs.end(); it++) {
        if (it->first >= cutoff) {
            it = bucket_refs.erase(it);
        } else {
            vector<int>::iterator vector_it;
            for (vector_it = it->second.begin(); vector_it !=  it->second.end(); ) {
                if (*vector_it >= cutoff) {
                    vector_it = it->second.erase(vector_it);
                } else {
                    ++vector_it;
                }
            }
            ++it;
        }
    }
};

// Insert an entry into its correct bucket
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

// Delete a reference of an entry relative to its adjacent entries
template<typename T>
void Directory<T>::delete_entry(T *entry) {
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

template class Directory<LRUBufferEntry>;
template class Directory<ClockBufferEntry>;

