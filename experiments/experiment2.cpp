#include <iostream>
#include <cassert>
#include "chrono"
#include "../include/SimpleKVStore.h"
#include "../include/util.h"
#include <algorithm>
#include <fstream>
#include <random>

using namespace std;

// Experiment 2.1: Compare the efficiency of LRU vs clock by measuring query throughput (on the y-axis)
// as a function of the maximum buffer pool size (on the x-axis) by designing an appropriate experiment.
// Try to identify one workload where LRU performs better and another workload where clock performs better.
//
// 1. Clock better than LRU:
// Uniformly randomly load and access keys in the database.
// The extra overhead associated with LRU should be enough to
// yield worst performance and this overhead is not payed off
// as keys are sampled uniformly random across entire DB
//
// 2. LRU better than Clock:
// Randomly load form a skewed sample and access keys in the database.
void experiment2p1(int num_MB, int step_size_MB) {
    cout << "Running Experiment 2.1" << endl;

    DbOptions *lru_options = new DbOptions();
    lru_options->setSSTSearch("BTree");
    lru_options->setSSTManager("BTreeManager");
    lru_options->setBufferPoolType("LRU");
    lru_options->setBufferPoolSize(1, num_MB);
    lru_options->setFilterBitsPerEntry(0);

    DbOptions *clock_options = new DbOptions();
    lru_options->setSSTSearch("BTree");
    lru_options->setSSTManager("BTreeManager");
    clock_options->setBufferPoolType("Clock");
    clock_options->setBufferPoolSize(1, num_MB);
    clock_options->setFilterBitsPerEntry(0);

    SimpleKVStore lru_db1;
    SimpleKVStore clock_db1;
    SimpleKVStore lru_db2;
    SimpleKVStore clock_db2;
    lru_db1.open("./experiments_dbs/lru_db1", lru_options);
    clock_db1.open("./experiments_dbs/clock_db1", clock_options);
    lru_db2.open("./experiments_dbs/lru_db2", lru_options);
    clock_db2.open("./experiments_dbs/clock_db2", clock_options);

    int max_buf_size_MB = num_MB;

    // Load 8 * max_buf_size bytes of entries in the database
    int num_inserts = 8 * max_buf_size_MB * MEGABYTE / ENTRY_SIZE;
    int num_queries = 0.00001 * num_inserts; // query 0.001% of data inserted

    std::vector<int> x;
    std::vector<double> lru_throughput1;
    std::vector<double> clock_throughput1;
    std::vector<double> lru_throughput2;
    std::vector<double> clock_throughput2;

    assert(step_size_MB * MEGABYTE < num_inserts);

    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<int> unif_sample(0, num_inserts);

    for (int i = 0; i < num_inserts; i++) {

        // Load db1s with uniformly randomly ordered keys
        int uniform = unif_sample(generator);
        clock_db1.put(uniform, 0);
        lru_db1.put(uniform, 0);

        int skew = ::rand() % num_inserts;
        // Load db2s with skewed keys
        clock_db2.put(skew, 0);
        lru_db2.put(skew, 0);
    }

    std::cout << "Generating " + to_string(max_buf_size_MB / step_size_MB) + " datapoints..." << std::endl;
    std::cout << "Iteration: ";

    for (int i = 0; i < max_buf_size_MB / step_size_MB; i++) {

        std::cout << to_string(i + 1) << " ";
        fflush(stdout);

        int buffer_size = (i + 1) * step_size_MB;

        clock_db1.set_buffer_pool_max_size(buffer_size);
        lru_db1.set_buffer_pool_max_size(buffer_size);
        clock_db2.set_buffer_pool_max_size(buffer_size);
        lru_db2.set_buffer_pool_max_size(buffer_size);

        // 1. === CLOCK BETTER ===

        // Make sure each experiment is querying consistently
        std::vector<int> queries(num_queries);
        for (int j = 0; j < num_queries; j++)
            queries[j] = unif_sample(generator); // Query uniformly randomly

        int val;

        // Time LRU
        auto start = chrono::high_resolution_clock::now();

        for (int j = 0; j < num_queries; j++)
            lru_db1.get(queries[j], (int &) val);

        auto stop = chrono::high_resolution_clock::now();
        double microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();

        lru_throughput1.push_back((1000 * (double) num_queries) / microsecs);

        // Time Clock
        start = chrono::high_resolution_clock::now();

        for (int j = 0; j < num_queries; j++)
            clock_db1.get(queries[j], (int &) val);

        stop = chrono::high_resolution_clock::now();
        microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();

        clock_throughput1.push_back((1000 * (double) num_queries) / microsecs);

        queries.clear();

        // 2. === LRU BETTER ===

        // Make sure each experiment is querying consistently
        for (int j = 0; j < num_queries; j++)
            queries[j] = ::rand() % num_inserts; // Query skewed randomly

        // Time LRU
        start = chrono::high_resolution_clock::now();

        for (int j = 0; j < num_queries; j++)
            lru_db2.get(queries[j], (int &) val);

        stop = chrono::high_resolution_clock::now();
        microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();

        lru_throughput2.push_back((1000 * (double) num_queries) / microsecs);

        // Time Clock
        start = chrono::high_resolution_clock::now();

        for (int j = 0; j < num_queries; j++)
            clock_db2.get(queries[j], (int &) val);

        stop = chrono::high_resolution_clock::now();
        microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();

        clock_throughput2.push_back((1000 * (double) num_queries) / microsecs);

        x.push_back((i + 1) * step_size_MB);
    }

    std::cout << std::endl;
    std::cout << "Printing to file..." << std::endl;

    assert(x.size() == lru_throughput1.size());
    assert(lru_throughput1.size() == clock_throughput1.size());
    assert(clock_throughput1.size() == lru_throughput2.size());
    assert(lru_throughput2.size() == clock_throughput2.size());

    std::ofstream exp2_data ("./experiments/data/exp2p1_data.csv");

    exp2_data << "Max Cache Size (MB),LRU Throughput,Clock Throughput,LRU Throughput,Clock Throughput" << std::endl;

    for (int i = 0; i < x.size(); i++) {
        exp2_data << x[i] << "," << to_string(lru_throughput1[i]) << "," << to_string(clock_throughput1[i]) << "," << to_string(lru_throughput2[i]) << "," << to_string(clock_throughput2[i]) << std::endl;
    }

    lru_db1.close();
    clock_db1.close();
    lru_db2.close();
    clock_db2.close();
}

// Experiment 2.2: Design an experiment comparing your binary search to B-tree search in terms of query
// throughput (on the y-axis) as you increase the data size (on the x-axis).
// This experiment should be done with uniformly randomly distributed queries and data.
void experiment2p2(int num_MB, int step_size_MB) {
    cout << "Running Experiment 2.2" << endl;

    DbOptions *btree_options = new DbOptions();
    btree_options->setSSTSearch("BTree");
    btree_options->setSSTManager("LSMTreeManager");
    btree_options->setBufferPoolType("None");
    btree_options->setFilterBitsPerEntry(0);

    DbOptions *bs_options = new DbOptions();
    bs_options->setSSTSearch("BinarySearch");
    bs_options->setSSTManager("LSMTreeManager");
    bs_options->setBufferPoolType("None");
    bs_options->setFilterBitsPerEntry(0);

    SimpleKVStore btree_db;
    SimpleKVStore bs_db;
    btree_db.open("./experiments_dbs/btree_db", btree_options);
    bs_db.open("./experiments_dbs/bs_db", bs_options);

    // Multiply size of int by two since we are inserting both a key and a value
    int num_inserts = num_MB * MEGABYTE / ENTRY_SIZE;
    int step_size = step_size_MB * MEGABYTE / ENTRY_SIZE;

    std::vector<int> x;
    std::vector<double> btree_throughput;
    std::vector<double> bs_throughput;

    assert(step_size < num_inserts);

    std::cout << "Generating " + to_string(num_inserts / step_size) + " datapoints..." << std::endl;
    std::cout << "Iteration: ";

    for (int i = 0; i < num_inserts / step_size; i++) {
        int db_num_keys = (i + 1) * step_size;

        std::cout << to_string(i + 1) << " ";
        fflush(stdout);

        // We assume that the time to generate random keys and manage the hash set is negligible
        for (int j = 0; j < step_size; j++) {
            int key = ::rand() % db_num_keys; // not necessarily uniformly distributed to simulate real workload (skewed towards lower keys)
            btree_db.put(key, 0);
            bs_db.put(key, 0);
        }

        int num_queries = 0.00001 * db_num_keys; // query 0.001% of data inserted

        // Make sure each experiment is querying consistently
        std::vector<int> queries(num_queries);
        for (int j = 0; j < num_queries; j++)
            queries[j] = ::rand() % num_inserts;

        int val;
        // Time and query Binary Search keys at random
        auto start = chrono::high_resolution_clock::now();

        for (int j = 0; j < num_queries; j++)
            btree_db.get(queries[j], (int &) val);

        auto stop = chrono::high_resolution_clock::now();
        double microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();

        btree_throughput.push_back((1000 * (double) num_queries) / microsecs);

        // Time and query BTree keys at random
        start = chrono::high_resolution_clock::now();

        for (int j = 0; j < num_queries; j++)
            bs_db.get(queries[j], (int &) val);

        stop = chrono::high_resolution_clock::now();
        microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();

        bs_throughput.push_back((1000 * (double) num_queries) / microsecs);

        x.push_back(db_num_keys * ENTRY_SIZE);
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
}
