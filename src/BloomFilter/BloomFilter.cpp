#include "BloomFilter.h"
#include "../../include/MurmurHash3.h"
#include "../../include/constants.h"
#include <iostream>

BloomFilter::BloomFilter(int num_entries, int bits_per_entry) {
    // generate set of random seeds
    int num_hash_functions = log(2) * bits_per_entry;
    while (seeds.size() != num_hash_functions) {
        seeds.insert(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::time_point_cast<std::chrono::milliseconds>(
                        std::chrono::high_resolution_clock::now()).time_since_epoch()).count());
    }
    bits = std::vector<int>(num_entries * bits_per_entry, 0);
}

BloomFilter::BloomFilter(int *buffer_data) {
    int num_seeds = buffer_data[0];
    buffer_data = buffer_data + 1; // skip 4 bytes
    for (int i = 0; i < num_seeds; i++) {
        this->seeds.insert(buffer_data[i]);
    }
    buffer_data += num_seeds;
    int bitmap_size = buffer_data[0];
    buffer_data = buffer_data + 1; // skip 1
    bits = std::vector<int>(*buffer_data, bitmap_size);
}


std::pair<int *, int> BloomFilter::serialize() {
    // num_seeds + data_size + seeds + bitmap
    int size = 2 + seeds.size() + bits.size();
    int *buf = new int[size];

    int index = 0;
    // write out seeds
    buf[index] = (int) seeds.size();
    index++;
    auto seeds_it = seeds.begin();
    int begin = index;
    for (; index < begin + buf[0]; index++) {
        buf[index] = *seeds_it;
        seeds_it++;
    }

    // write out bitmap
    auto bits_it = bits.begin();
    buf[index] = (int) bits.size();
    index++;
    begin = index;
    for (; index < begin + bits.size(); index++) {
        buf[index] = *bits_it;
        bits_it++;
    }

    int end = index + PAGE_SIZE - (index % PAGE_SIZE);
    for (; index < end && index < size; index++) {
        buf[index] = INT_MAX;
    }

    return std::pair(buf, ceil((double) index / PAGE_SIZE));
}


void BloomFilter::insert(int key) {
    for (auto seed : seeds) {
        long hash;
        MurmurHash3_x86_32((void *) &key, sizeof(int), seed, (void *) &hash);
        bits[hash % bits.size()] = 1;
    }
}

bool BloomFilter::testMembership(int key) {

    for (auto seed : seeds) {
        long hash;
        MurmurHash3_x86_32((void *) &key, sizeof(int), seed, (void *) &hash);
        if (bits[hash % bits.size()] == 0) {
            return false;
        }
    }
    return true;
}