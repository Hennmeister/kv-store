#include <iostream>
#include <cassert>
#include "chrono"
#include "../include/SimpleKVStore.h"
#include "../include/constants.h"
#include "../include/Base/DbOptions.h"
#include "../include/util.h"
#include <algorithm>
#include <unordered_set>
#include <random>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sys/stat.h>

using namespace std;

// Experiment 3.1: Measure insertion, get, and scan throughput for your implementation over time as the data size
// grows. Describe your experimental setup and make sure all relevant variables are controlled. Please fix the buffer
// pool size to 10 MB, the Bloom filters to use 5 bits per entry, and the memtable to 1 MB. Run this experiment as you
// insert 1 GB of data. Measure get and scan throughput at regular intervals as you insert this data.
void experiment3p1(int num_MB, int step_size) {
    cout << "Running Experiment 3.1" << endl;

    DbOptions *options = new DbOptions();
    options->setSSTSearch("LSMTree");
    options->setBufferPoolSize(0, 10);
 //   options->setBloomBits(5);
    options->setMaxMemtableSize(1 * MEGABYTE);

    SimpleKVStore db;
    db.open("./experiments_dbs/experiment_3p1"); // TODO: specs

    // Load 1 GB of data
    long long int num_inserts = 1024 * MEGABYTE / ENTRY_SIZE;

    std::vector<long long int> x;
    std::vector<long long int> puts_throughput;
    std::vector<long long int> gets_throughput;
    std::vector<long long int> scans_throughput;

    int num_samples = 1.2 * num_inserts; // take a few extra samples to cover repeated puts
    long long int key_max = 50 * num_inserts; // add some randomness

    assert(key_max < INT_MAX);
    assert(step_size < num_inserts);

    long long int rand_keys[num_samples];
    for (int j = 0; j < num_samples; j++) {
        rand_keys[j] = ::rand() % key_max; // not necessarily uniformly distributed to simulate real workload (skewed towards lower keys)
    }

    std::unordered_set<long long int> unique_keys;

    std::cout << "Generating " + to_string(num_inserts / step_size) + " datapoints..." << std::endl;
    std::cout << "Iteration: ";

    long long int offset = 0;
    for (int i = 0; i < num_inserts / step_size; i++) {

        std::cout << to_string(i + 1) << " ";
        fflush(stdout);

        // Check how many puts are needed to reach (i + 1) * step_size
        // unique keys in the db (might do some duplicate puts)
        // Do this computation here to prevent set operations from affecting
        // timed measurements
        int puts_count = 0;
        while (unique_keys.size() < (i + 1) * step_size) {
            long long int key = rand_keys[offset + puts_count];
            if (unique_keys.find(key) == unique_keys.end()) {
                unique_keys.insert(key);
            }
            puts_count++;
        }

        auto start = chrono::high_resolution_clock::now();

        // actually run and time correct number of puts
        for (int j = 0; j < puts_count; j++) {
            db.put(rand_keys[offset + j], 0); // paylod is irrelevant
        }
        auto stop = chrono::high_resolution_clock::now();
        double microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();
        puts_throughput.push_back((1000 * (double)puts_count) / microsecs);

        offset += puts_count;

        // generate step_size random keys from the unique inserted keys
        long long int rand_gets[step_size];
        for (int j = 0; j < step_size; j++) {
            long long int key;
            std::mt19937 gen(std::random_device{}());
            std::sample(unique_keys.begin(), unique_keys.end(), &key, 1, gen);
            rand_gets[j] = key;
        }

        // step_size number of random gets
        int val;
        start = chrono::high_resolution_clock::now();
        for (int j = 0; j < step_size; j++)
            db.get(rand_gets[j], (int &) val);

        stop = chrono::high_resolution_clock::now();
        microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();
        gets_throughput.push_back((1000 * (double)step_size) / microsecs);

        // step_size number of random scans
        start = chrono::high_resolution_clock::now();
        for (int j = 1; j < step_size; j++)
            db.scan(min(rand_gets[j-1], rand_gets[j]), max(rand_gets[j-1], rand_gets[j]));

        stop = chrono::high_resolution_clock::now();
        microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();
        scans_throughput.push_back((1000 * (double)step_size) / microsecs);

        x.push_back(unique_keys.size() * ENTRY_SIZE);
    }

    std::cout << std::endl;
    std::cout << "Printing to file..." << std::endl;

    assert(x.size() == puts_throughput.size());
    assert(puts_throughput.size() == gets_throughput.size());
    assert(gets_throughput.size() == scans_throughput.size());

    std::ofstream exp3_data ("./experiments/data/exp3p1_data.csv");

    exp3_data << "Inserted data (Bytes),Puts Throughput (operations/msec),Gets Throughput (operations/msec),Scans Throughput (operations/msec)" << std::endl;

    for (int i = 0; i < x.size(); i++) {
        exp3_data << x[i] << "," << to_string(puts_throughput[i]) << "," << to_string(gets_throughput[i]) << "," << to_string(scans_throughput[i]) << std::endl;
    }

    db.close();

    // Clear experiment db
    for (const auto &entry : std::filesystem::directory_iterator("./experiments_dbs"))
        std::filesystem::remove_all(entry.path());
}


// Experiment 3.2: Illustrate an experiment showing get performance as you vary the number of bits per entry used for
// your Bloom filters.
void experiment3p2(int max_M, int step_size) {
    cout << "Running Experiment 3.2" << endl;

    // Load 1 GB of data on each run
    long long int num_inserts = 1024 * MEGABYTE / ENTRY_SIZE;
    long long int num_queries = 0.01 * num_inserts; // query 1% of db size

    std::vector<long long int> x;
    std::vector<long long int> get_throughput;

    assert(step_size < num_inserts);

    std::vector<long long int> rand_keys(num_inserts);

    // fills vector with increasing keys starting from 1
    std::iota(rand_keys.begin(), rand_keys.end(), 1);

    std::random_device rd;
    std::mt19937 gen(rd());

    // Generate shuffled insert order
    std::shuffle(rand_keys.begin(), rand_keys.end(), gen);

    std::cout << "Generating " + to_string(max_M / step_size) + " datapoints..." << std::endl;
    std::cout << "Iteration: ";

    for (int i = 0; i < max_M / step_size; i++) {
        int M = (i + 1) * step_size;
        std::cout << to_string(i + 1) << " ";
        fflush(stdout);

        // TODO: set bits per entry here

        SimpleKVStore db;
        db.open("./experiments_dbs/exp3p2_" + to_string(M) + "bits_per_entry");

        // Load db with randomly ordered keys
        for (int i = 0; i < rand_keys.size(); i++)
            db.put(rand_keys[i], 0);

        // Time queries
        int val;
        auto start = chrono::high_resolution_clock::now();

        for (int i = 0; i < num_queries; i++)
            db.get(rand_int(1, num_queries), (int &) val);

        auto stop = chrono::high_resolution_clock::now();
        double microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();

        get_throughput.push_back((1000 * (double) num_queries) / microsecs);

        x.push_back(M);

        db.close();
    }

    std::cout << std::endl;
    std::cout << "Printing to file..." << std::endl;

    assert(x.size() == get_throughput.size());

    std::ofstream exp3_data ("./experiments/data/exp3p2_data.csv");

    exp3_data << "Bloom filter bits per entry (M),Get Throughput (operations/msec)" << std::endl;

    for (int i = 0; i < x.size(); i++) {
        exp3_data << x[i] << "," << to_string(get_throughput[i]) << std::endl;
    }

    // Clear experiment db
    for (const auto &entry : std::filesystem::directory_iterator("./experiments_dbs"))
        std::filesystem::remove_all(entry.path());
}