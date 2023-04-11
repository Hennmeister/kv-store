#include <iostream>
#include <cassert>
#include "chrono"
#include "../include/SimpleKVStore.h"
#include "../include/constants.h"
#include "../include/Base/DbOptions.h"
#include "../include/util.h"
#include <algorithm>
#include <random>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sys/stat.h>
#include <unordered_set>

using namespace std;

// Experiment 2.1: Compare the efficiency of LRU vs clock by measuring query throughput (on the y-axis)
// as a function of the maximum buffer pool size (on the x-axis) by designing an appropriate experiment.
// Try to identify one workload where LRU performs better and another workload where clock performs better.
//
// 1. Clock better than LRU:
// Uniformly and randomly load and acess keys in the database.
// The extra overhead associated with LRU should be enough to
// yield worst performance and this overhead is not payed off
// as keys are sampled uniformly random across entire DB
//
// 2. LRU better than Clock:
// Load keys in order so we can have a sense of spacial locality.
// Access keys in a way that we benefit from the overhead of LRU
// putting the page to the front while clock evicts page earlier
// since it just sets a bit that is already 1 to 1 again (effectively doing nothing).
// For that, we have to access a page once to put it in cache,
// and then access it again fast enough so that the clock did
// not reach said page and changed to 0, while still trying to
// wait as long as possible so that LRU can still keep the page
// while clock evicted on a third access
void experiment2p1(int num_MB, int step_size) {
    cout << "Running Experiment 2.1" << endl;

    DbOptions *lru_options = new DbOptions();
    lru_options->setBufferPoolType("LRU");
    lru_options->setBufferPoolSize(1, num_MB);

    DbOptions *clock_options = new DbOptions();
    clock_options->setBufferPoolType("Clock");
    clock_options->setBufferPoolSize(1, num_MB);

    SimpleKVStore lru_db1;
    SimpleKVStore clock_db1;
    SimpleKVStore lru_db2;
    SimpleKVStore clock_db2;
    lru_db1.open("./experiments_dbs/lru_db", lru_options);
    clock_db1.open("./experiments_dbs/clock_db", clock_options);
    lru_db2.open("./experiments_dbs/lru_db", lru_options);
    clock_db2.open("./experiments_dbs/clock_db", clock_options);

    int max_buf_size = num_MB * MEGABYTE;

    // Load 8 * max_buf_size bytes of entries in the database
    int num_inserts = 8 * max_buf_size / ENTRY_SIZE;
    int num_queries = 0.00001 * num_inserts; // query 0.001% of data inserted

    std::cout << "Averaging from " + to_string(num_queries) + " queries" << std::endl;

    std::vector<int> x;
    std::vector<double> lru_throughput1;
    std::vector<double> clock_throughput1;
    std::vector<double> lru_throughput2;
    std::vector<double> clock_throughput2;

    assert(step_size < num_inserts);

    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<int> unif_sample(0, num_inserts);

    std::unordered_set<int> unique_keys;
    // Load db with uniformly random keys until num_inserts
    while (unique_keys.size() < num_inserts) {
        int key = unif_sample(generator);
        if (unique_keys.find(key) == unique_keys.end()) {
            unique_keys.insert(key);
            // Load db1s with randomly ordered keys
            clock_db1.put(key, 0);
            lru_db1.put(key, 0);
        }
    }

    for (int i = 0; i < num_inserts; i++) {
        // Load db2s with sequential keys
        clock_db2.put(i, 0);
        lru_db2.put(i, 0);
    }

    std::cout << "Generating " + to_string(max_buf_size / step_size) + " datapoints..." << std::endl;
    std::cout << "Iteration: ";

    for (int i = 0; i < max_buf_size / step_size; i++) {

        std::cout << to_string(i + 1) << " ";
        fflush(stdout);

        int buffer_size = (i + 1) * step_size;

        clock_db1.set_buffer_pool_max_size(buffer_size);;
        lru_db1.set_buffer_pool_max_size(buffer_size);;
        clock_db2.set_buffer_pool_max_size(buffer_size);;
        lru_db2.set_buffer_pool_max_size(buffer_size);;

        // 1. === CLOCK BETTER ===

        int val;

        // Time LRU
        auto start = chrono::high_resolution_clock::now();

        for (int i = 0; i < num_queries; i++)
            lru_db1.get(rand_int(1, num_inserts), (int &) val);

        auto stop = chrono::high_resolution_clock::now();
        double microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();

        lru_throughput1.push_back((1000 * (double) num_queries) / microsecs);

        // Time Clock
        start = chrono::high_resolution_clock::now();

        for (int i = 0; i < num_queries; i++)
            clock_db1.get(rand_int(1, num_inserts), (int &) val);

        stop = chrono::high_resolution_clock::now();
        microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();

        clock_throughput1.push_back((1000 * (double) num_queries) / microsecs);

        // 2. === LRU BETTER ===

        // Set key range we are iterating over

        int num_pages_in_buf = buffer_size / PAGE_SIZE;

        // Reaccess the same page after querying 30% of the buffer size new pages
        // This tries to be long enough so that clock handle did not tick
        int reaccess_after = 0.30 * num_pages_in_buf; // take the floor of 30%

        // Get keys PAGE_NUM_ENTRIES apart to gurantee different entries
        int start_key = rand_int(1, num_inserts - (reaccess_after * PAGE_NUM_ENTRIES));

        std::vector<int> keys_iterated = {start_key};
        for (int i = 0; i < reaccess_after; i++)
            keys_iterated.push_back(start_key + i * PAGE_NUM_ENTRIES);

        // Time LRU
        start = chrono::high_resolution_clock::now();

        for (int i = 0; i < num_queries; i++) {
            // To avoid synchronizing with the clock handle, query from
            // random key after 10% of buffer size queries happened
            if (i % (int)(0.1 * num_pages_in_buf) == 0) {
                lru_db2.get(rand_int(1, num_inserts), (int &) val);
            } else {
                lru_db2.get(keys_iterated[i % reaccess_after + 1], (int &) val);
            }
        }

        stop = chrono::high_resolution_clock::now();
        microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();

        lru_throughput2.push_back((1000 * (double) num_queries) / microsecs);

        // Time Clock
        start = chrono::high_resolution_clock::now();

        for (int i = 0; i < num_queries; i++) {
            // To avoid synchronizing with the clock handle, query from
            // random key after 10% of buffer size queries happened
            if (i % (int)(0.1 * num_pages_in_buf) == 0) {
                clock_db2.get(rand_int(1, num_inserts), (int &) val);
            } else {
                clock_db2.get(keys_iterated[i % reaccess_after + 1], (int &) val);
            }
        }

        stop = chrono::high_resolution_clock::now();
        microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();

        clock_throughput2.push_back((1000 * (double) num_queries) / microsecs);

        x.push_back((i + 1) * step_size * ENTRY_SIZE);
    }

    std::cout << std::endl;
    std::cout << "Printing to file..." << std::endl;

    assert(x.size() == lru_throughput1.size());
    assert(lru_throughput1.size() == clock_throughput1.size());
    assert(clock_throughput1.size() == lru_throughput2.size());
    assert(lru_throughput2.size() == clock_throughput2.size());

    std::ofstream exp2_data ("./experiments/data/exp2p1_data.csv");

    exp2_data << "Max Cache Size (Bytes),LRU Throughput,Clock Throughput,LRU Throughput,Clock Throughput" << std::endl;

    for (int i = 0; i < x.size(); i++) {
        exp2_data << x[i] << "," << to_string(lru_throughput1[i]) << "," << to_string(clock_throughput1[i]) << "," << to_string(lru_throughput2[i]) << "," << to_string(clock_throughput2[i]) << std::endl;
    }

    lru_db1.close();
    clock_db1.close();
    lru_db2.close();
    clock_db2.close();

    // Clear experiment db
    for (const auto &entry : std::filesystem::directory_iterator("./experiments_dbs"))
        std::filesystem::remove_all(entry.path());
}

// Experiment 2.2: Design an experiment comparing your binary search to B-tree search in terms of query
// throughput (on the y-axis) as you increase the data size (on the x-axis).
// This experiment should be done with uniformly randomly distributed queries and data.
void experiment2p2(int num_MB, int step_size_MB) {
    cout << "Running Experiment 2.2" << endl;

    DbOptions *btree_options = new DbOptions();
    btree_options->setSSTSearch("BTree");

    DbOptions *bs_options = new DbOptions();
    bs_options->setSSTSearch("BinarySearch");

    SimpleKVStore btree_db;
    SimpleKVStore bs_db;
    btree_db.open("./experiments_dbs/btree_db", btree_options);
    bs_db.open("./experiments_dbs/bs_db", bs_options);

    // Multiply size of int by two since we are inserting both a key and a value
    int num_inserts = num_MB * MEGABYTE / ENTRY_SIZE;
    int step_size = step_size_MB * MEGABYTE / ENTRY_SIZE;
    int num_queries = 0.00001 * num_inserts; // query 0.001% of data inserted

    std::cout << "Averaging from " + to_string(num_queries) + " queries" << std::endl;

    std::vector<int> x;
    std::vector<double> btree_throughput;
    std::vector<double> bs_throughput;

    assert(step_size < num_inserts);

    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<int> unif_sample(0, num_inserts);

    std::cout << "Inserting " + to_string(num_inserts) + " keys (percent included): ";

    std::unordered_set<int> unique_keys;
    // Load db with uniformly random keys until num_inserts
    while (unique_keys.size() < num_inserts) {
        int key = unif_sample(generator);
        if (unique_keys.find(key) == unique_keys.end()) {
            unique_keys.insert(key);
            btree_db.put(key, 0);
            bs_db.put(key, 0);
        }
    }

    std::cout << "Generating " + to_string(num_inserts / step_size) + " datapoints..." << std::endl;
    std::cout << "Iteration: ";

    for (int i = 0; i < num_inserts / step_size; i++) {

        std::cout << to_string(i + 1) << " ";
        fflush(stdout);

        int val;
        // Time and query Binary Search keys uniformly at random
        auto start = chrono::high_resolution_clock::now();

        for (int i = 0; i < num_queries; i++)
            btree_db.get(rand_int(1, num_inserts), (int &) val);

        auto stop = chrono::high_resolution_clock::now();
        double microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();

        btree_throughput.push_back((1000 * (double) num_queries) / microsecs);

        // Time and query BTree keys uniformly at random
        start = chrono::high_resolution_clock::now();

        for (int i = 0; i < num_queries; i++)
            bs_db.get(rand_int(1, num_inserts), (int &) val);

        stop = chrono::high_resolution_clock::now();
        microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();

        bs_throughput.push_back((1000 * (double) num_queries) / microsecs);

        x.push_back((i + 1) * step_size * ENTRY_SIZE);
    }

    std::cout << std::endl;
    std::cout << "Printing to file..." << std::endl;

    assert(x.size() == btree_throughput.size());
    assert(btree_throughput.size() == bs_throughput.size());

    std::ofstream exp2_data ("./experiments/data/exp2p2_data.csv");

    exp2_data << "Inserted data (Bytes),BTree Search,Binary Search" << std::endl;

    for (int i = 0; i < x.size(); i++) {
        exp2_data << x[i] << "," << to_string(btree_throughput[i]) << "," << to_string(bs_throughput[i]) << std::endl;
    }

    btree_db.close();
    bs_db.close();

    // Clear experiment db
    for (const auto &entry : std::filesystem::directory_iterator("./experiments_dbs"))
        std::filesystem::remove_all(entry.path());
}