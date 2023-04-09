#ifndef KV_STORE_BLOOMFILTER_H
#define KV_STORE_BLOOMFILTER_H

#include <cmath>
#include "set"
#include "chrono"
#include "bitset"
#include "vector"


class BloomFilter {

public:
    BloomFilter(int num_entries, int bits_per_entry);
    void insert(std::string key);
    bool testMembership(std::string key);

private:
    std::vector<bool> bits; // TODO: look into using boost::dynamic_bitset
    std::set<int> seeds;
};


#endif //KV_STORE_BLOOMFILTER_H
