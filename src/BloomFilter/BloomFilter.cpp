#include "BloomFilter.h"
#include "../../include/MurmurHash3.h"
#include <iostream>

BloomFilter::BloomFilter(int num_entries, int bits_per_entry) {
    // generate set of random seeds
    int num_hash_functions = log(bits_per_entry);
    while (seeds.size() != num_hash_functions) {
        seeds.insert(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::time_point_cast<std::chrono::milliseconds>(
                        std::chrono::high_resolution_clock::now()).time_since_epoch()).count());
    }
    bits = std::vector<bool>(num_entries * bits_per_entry, false);
}

void BloomFilter::insert(std::string key) {
    for (auto seed : seeds) {
        uint32_t hash;
        MurmurHash3_x86_32(key.c_str(), key.size(), seed, &hash);
        bits[hash % bits.size()] = true;
    }
}

bool BloomFilter::testMembership(std::string key) {
    for (auto seed : seeds) {
        uint32_t hash;
        MurmurHash3_x86_32(key.c_str(), key.size(), seed, &hash);
        if (bits[hash % bits.size()] == false) {
            return false;
        }
    }
    return true;
}
