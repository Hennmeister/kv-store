#include "BloomFilter.h"
#include "../../include/MurmurHash3.h"
#include "../../include/constants.h"
#include <climits>
#include "random"

using namespace std;

BloomFilter::BloomFilter(int num_entries, int bits_per_entry) {
    // generate set of random seeds
    int num_hash_functions = log(2) * bits_per_entry;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned int> dis;
    while (seeds.size() != num_hash_functions) {
        seeds.insert(dis(gen));
    }
    bitmap_size = ceil((double) num_entries * bits_per_entry / (sizeof(int) * 8));
    bits = new int[bitmap_size]{0};
    data_buf = nullptr;
}

BloomFilter::BloomFilter(int *buffer_data) {
    this->data_buf = buffer_data;
    int num_seeds = buffer_data[0];
    buffer_data = buffer_data + 1; // skip 4 bytes
    for (int i = 0; i < num_seeds; i++) {
        this->seeds.insert(buffer_data[i]);
    }
    buffer_data += num_seeds;
    bitmap_size = buffer_data[0];
    buffer_data = buffer_data + 1; // skip 1
    this->bits = buffer_data;
}


std::pair<int *, int> BloomFilter::serialize() {
    // num_seeds + data_size + seeds + bitmap
    int size = ceil(double ((2 + seeds.size() + bitmap_size) *  sizeof(int))/ (double) PAGE_SIZE) * (PAGE_SIZE/sizeof(int));
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
    buf[index] = bitmap_size;
    index++;

    std::memcpy(&buf[index], bits, bitmap_size);
    index += bitmap_size;

    int end = index + PAGE_SIZE - (index % PAGE_SIZE);
    for (; index < end && index < size; index++) {
        buf[index] = INT_MAX;
    }

    return std::pair(buf, size * sizeof(int) / PAGE_SIZE);
}


void BloomFilter::insert(int key) {
    for (auto seed : seeds) {
        int hash;
        MurmurHash3_x86_32((void *) &key, sizeof(int), seed, (void *) &hash);
        hash = hash % (bitmap_size * sizeof(int) * 8);
        bits[hash / (sizeof(int) * 8)] |= 1 << (hash % (sizeof(int) * 8));
    }
}


bool BloomFilter::testMembership(int key) {

    for (auto seed : seeds) {
        int hash;
        MurmurHash3_x86_32((void *) &key, sizeof(int), seed, (void *) &hash);
        hash = hash % (bitmap_size * sizeof(int) * 8);
        if ((bits[hash / (sizeof(int) * 8)] & (1 << (hash % (sizeof(int) * 8)))) == 0) {
            return false;
        }
    }
    return true;
}

BloomFilter::~BloomFilter() {
    if (data_buf != nullptr)
        delete[] data_buf;
}
