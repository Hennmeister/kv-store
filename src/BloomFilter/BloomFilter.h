#ifndef KV_STORE_BLOOMFILTER_H
#define KV_STORE_BLOOMFILTER_H

#include <cmath>
#include "set"
#include "chrono"
#include "bitset"
#include "vector"


class BloomFilter {

public:
    // Used to create a new bloom filter
    BloomFilter(int num_entries, int bits_per_entry);
    // Used to recreate a previously existing filter when reading in data from storage
    BloomFilter(int *buffer_data);
    // serialize this filter into buffer data that can be read to initialize an identical bloom filter using the above constructor
    // Note: you must free the returned buffer, and the size does not include padded data
    std::pair<int *, int> serialize();
    void insert(int key);
    bool testMembership(int key);

private:
    // note that we use int for ease of serialization
    std::vector<int> bits; // TODO: look into using boost::dynamic_bitset
    std::set<int> seeds;
};


#endif //KV_STORE_BLOOMFILTER_H
