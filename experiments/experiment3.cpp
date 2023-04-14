#include <iostream>
#include <cassert>
#include "chrono"
#include "../include/SimpleKVStore.h"
#include "../include/util.h"
#include <algorithm>
#include <fstream>
#include <filesystem>

using namespace std;

// Experiment 3.1: Measure insertion, get, and scan throughput for your implementation over time as the data size
// grows. Describe your experimental setup and make sure all relevant variables are controlled. Please fix the buffer
// pool size to 10 MB, the Bloom filters to use 5 bits per entry, and the memtable to 1 MB. Run this experiment as you
// insert 1 GB of data. Measure get and scan throughput at regular intervals as you insert this data.
void experiment3p1(int num_MB, int step_size_MB) {
    cout << "Running Experiment 3.1" << endl;

    DbOptions *options = new DbOptions();
    options->setSSTSearch("BTree");
    options->setSSTManager("LSMTreeManager");
    options->setBufferPoolType("Clock");
    options->setBufferPoolSize(1, 10);
    options->setFilterBitsPerEntry(5);
    options->setMaxMemtableSize(1 * MEGABYTE);

    SimpleKVStore db;
    db.open("./experiments_dbs/experiment_3p1", options);

    assert(num_MB == 1024);

    // Load 1 GB of data
    int num_inserts = num_MB * MEGABYTE / ENTRY_SIZE;
    int step_size = step_size_MB * MEGABYTE / ENTRY_SIZE;

    std::vector<int> x;
    std::vector<double> puts_throughput;
    std::vector<double> gets_throughput;
    std::vector<double> scans_throughput;

    assert(step_size < num_inserts);

    std::cout << "Generating " + to_string(num_inserts / step_size) + " datapoints..." << std::endl;
    std::cout << "Iteration: ";

    for (int i = 0; i < num_inserts / step_size; i++) {
        int db_num_keys = (i + 1) * step_size;

        std::cout << to_string(i + 1) << " ";
        fflush(stdout);

        auto start = chrono::high_resolution_clock::now();

        // We assume that the time to generate random keys and manage the hash set is negligible
        for (int j = 0; j < step_size; j++) {
            int key = ::rand() % db_num_keys; // not necessarily uniformly distributed to simulate real workload (skewed towards lower keys)
            db.put(key, 0); // paylod is irrelevant
        }

        auto stop = chrono::high_resolution_clock::now();
        double microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();
        puts_throughput.push_back((1000 * (double)step_size) / microsecs);

        int num_queries = 0.000005 * db_num_keys; // query 0.0005% of data inserted

        // num_queries number of random gets
        int val;
        start = chrono::high_resolution_clock::now();
        for (int j = 0; j < num_queries; j++)
            db.get(::rand() % db_num_keys, (int &) val);

        stop = chrono::high_resolution_clock::now();
        microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();
        gets_throughput.push_back((1000 * (double)num_queries) / microsecs);

        // num_queries number of random scans
        start = chrono::high_resolution_clock::now();
        for (int j = 1; j < num_queries; j++) {
            int k1 = ::rand() % db_num_keys;
            int k2 = ::rand() % db_num_keys;
            db.scan(min(k1, k2), max(k1, k2));
        }

        stop = chrono::high_resolution_clock::now();
        microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();
        scans_throughput.push_back((1000 * (double)num_queries) / microsecs);

        x.push_back(db_num_keys * ENTRY_SIZE);
    }

    std::cout << std::endl;
    std::cout << "Printing to file..." << std::endl;

    assert(x.size() == puts_throughput.size());
    assert(puts_throughput.size() == gets_throughput.size());
    assert(gets_throughput.size() == scans_throughput.size());

    std::ofstream exp3_data ("./experiments/data/exp3p1_data.csv");

    exp3_data << "Inserted data (Bytes),Puts Throughput,Gets Throughput,Scans Throughput" << std::endl;

    for (int i = 0; i < x.size(); i++) {
        exp3_data << x[i] << "," << to_string(puts_throughput[i]) << "," << to_string(gets_throughput[i]) << "," << to_string(scans_throughput[i]) << std::endl;
    }

    db.close();
}


// Experiment 3.2: Illustrate an experiment showing get performance as you vary the number of bits per entry used for
// your Bloom filters.
void experiment3p2(int max_M, int step_size) {
    cout << "Running Experiment 3.2" << endl;

    // Load 256 MB of data on each run
    int num_inserts = 256 * MEGABYTE / ENTRY_SIZE;
    int num_queries = 0.0001 * num_inserts; // query 0.01% of data inserted

    // Make sure each experiment is inserting consistently
    std::vector<int> inserts(num_inserts);
    for (int j = 0; j < num_inserts; j++)
        inserts[j] = ::rand() % num_inserts;

    // Make sure each experiment is querying consistently
    std::vector<int> queries(num_queries);
    for (int j = 0; j < num_queries; j++)
        queries[j] = ::rand() % num_inserts;

    std::vector<int> x;
    std::vector<double> get_throughput;

    assert(step_size < num_inserts);

    std::cout << "Generating " + to_string(max_M / step_size) + " datapoints..." << std::endl;
    std::cout << "Iteration: ";

    for (int i = 0; i < max_M / step_size; i++) {
        int M = (i + 1) * step_size;
        std::cout << to_string(i + 1) << " ";
        fflush(stdout);

        DbOptions *options = new DbOptions();
        options->setSSTSearch("BTree");
        options->setSSTManager("LSMTreeManager");
        options->setBufferPoolType("Clock");
        options->setBufferPoolSize(20, 20);
        options->setMaxMemtableSize(1 * MEGABYTE);
        options->setFilterBitsPerEntry(M);

        SimpleKVStore db;
        db.open("./experiments_dbs/exp3p2_" + to_string(M) + "bits_per_entry", options);

        // Load db with random keys until num_inserts
        for (int j = 0; j < num_inserts; j++)
            db.put(inserts[j], 0); // paylod is irrelevant

        // Time queries
        int val;
        auto start = chrono::high_resolution_clock::now();

        for (int j = 0; j < num_queries; j++)
            db.get(queries[j], (int &) val);

        auto stop = chrono::high_resolution_clock::now();
        double microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();

        get_throughput.push_back((1000 * (double) num_queries) / microsecs);

        x.push_back(M);

        db.close();

        // Clear experiment db
        for (const auto &entry : std::filesystem::directory_iterator("./experiments_dbs"))
            std::filesystem::remove_all(entry.path());
    }

    std::cout << std::endl;
    std::cout << "Printing to file..." << std::endl;

    assert(x.size() == get_throughput.size());

    std::ofstream exp3_data ("./experiments/data/exp3p2_data.csv");

    exp3_data << "Bloom filter bits per entry (M),Gets Throughput (operations/msec)" << std::endl;

    for (int i = 0; i < x.size(); i++) {
        exp3_data << x[i] << "," << to_string(get_throughput[i]) << std::endl;
    }

}