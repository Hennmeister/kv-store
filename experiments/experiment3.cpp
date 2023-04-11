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
void experiment3p1(int num_MB, int step_size_MB) {
    cout << "Running Experiment 3.1" << endl;

    DbOptions *options = new DbOptions();
    options->setSSTSearch("LSMTree");
    options->setBufferPoolSize(0, 10);
    options->setFilterBitsPerEntry(5);
    options->setMaxMemtableSize(1 * MEGABYTE);

    SimpleKVStore db;
    db.open("./experiments_dbs/experiment_3p1", options);

    assert(num_MB == 1024);

    // Load 1 GB of data
    int num_inserts = num_MB * MEGABYTE / ENTRY_SIZE;
    int num_queries = 0.00001 * num_inserts; // query 0.001% of data inserted
    int step_size = step_size_MB * MEGABYTE / ENTRY_SIZE;

    std::cout << "Averaging from " + to_string(num_queries) + " queries" << std::endl;

    std::vector<int> x;
    std::vector<double> puts_throughput;
    std::vector<double> gets_throughput;
    std::vector<double> scans_throughput;

    int key_max = 50 * num_inserts; // add some randomness

    assert(key_max < INT_MAX);
    assert(step_size < num_inserts);

    std::unordered_set<int> unique_keys;

    std::cout << "Generating " + to_string(num_inserts / step_size) + " datapoints..." << std::endl;
    std::cout << "Iteration: ";

    int offset = 0;
    for (int i = 0; i < num_inserts / step_size; i++) {

        std::cout << to_string(i + 1) << " ";
        fflush(stdout);

        auto start = chrono::high_resolution_clock::now();

        // We ensure that the duplicated keys are not counted as data inserted
        // We also assume that the time to generate random keys and manage the hash set is negligible
        int puts_count = 0;
        while (unique_keys.size() < (i + 1) * step_size) {
            int key = ::rand() % key_max; // not necessarily uniformly distributed to simulate real workload (skewed towards lower keys);
            if (unique_keys.find(key) == unique_keys.end()) {
                unique_keys.insert(key);
                db.put(key, 0); // paylod is irrelevant
            }
            puts_count++;
        }

        auto stop = chrono::high_resolution_clock::now();
        double microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();
        puts_throughput.push_back((1000 * (double)puts_count) / microsecs);

        offset += puts_count;

        // generate num_queries random keys from the unique inserted keys
        int rand_gets[num_queries];
        for (int j = 0; j < num_queries; j++) {
            int key;
            std::mt19937 gen(std::random_device{}());
            std::sample(unique_keys.begin(), unique_keys.end(), &key, 1, gen);
            rand_gets[j] = key;
        }

        // num_queries number of random gets
        int val;
        start = chrono::high_resolution_clock::now();
        for (int j = 0; j < num_queries; j++)
            db.get(rand_gets[j], (int &) val);

        stop = chrono::high_resolution_clock::now();
        microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();
        gets_throughput.push_back((1000 * (double)num_queries) / microsecs);

        // num_queries number of random scans
        start = chrono::high_resolution_clock::now();
        for (int j = 1; j < num_queries; j++)
            db.scan(min(rand_gets[j-1], rand_gets[j]), max(rand_gets[j-1], rand_gets[j]));

        stop = chrono::high_resolution_clock::now();
        microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();
        scans_throughput.push_back((1000 * (double)num_queries) / microsecs);

        x.push_back(unique_keys.size() * ENTRY_SIZE);
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

    // Clear experiment db
    for (const auto &entry : std::filesystem::directory_iterator("./experiments_dbs"))
        std::filesystem::remove_all(entry.path());
}


// Experiment 3.2: Illustrate an experiment showing get performance as you vary the number of bits per entry used for
// your Bloom filters.
void experiment3p2(int max_M, int step_size) {
    cout << "Running Experiment 3.2" << endl;

    // Load 1 GB of data on each run
    int num_inserts = 16 * MEGABYTE / ENTRY_SIZE;
    int num_queries = 0.00001 * num_inserts; // query 0.001% of data inserted

    std::cout << "Averaging from " + to_string(num_queries) + " queries" << std::endl;

    std::vector<int> x;
    std::vector<double> get_throughput;

    assert(step_size < num_inserts);

    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<int> unif_sample(0, num_inserts);

    std::cout << "Generating " + to_string(max_M / step_size) + " datapoints..." << std::endl;
    std::cout << "Iteration: ";

    for (int i = 0; i < max_M / step_size; i++) {
        int M = (i + 1) * step_size;
        std::cout << to_string(i + 1) << " ";
        fflush(stdout);

        DbOptions *options = new DbOptions();
        options->setFilterBitsPerEntry(M);

        SimpleKVStore db;
        db.open("./experiments_dbs/exp3p2_" + to_string(M) + "bits_per_entry", options);

        std::unordered_set<int> unique_keys;
        // Load db with uniformly random keys until num_inserts
        while (unique_keys.size() < num_inserts) {
            int key = unif_sample(generator);
            if (unique_keys.find(key) == unique_keys.end()) {
                unique_keys.insert(key);
                db.put(key, 0); // paylod is irrelevant
            }
        }

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

    exp3_data << "Bloom filter bits per entry (M),Gets Throughput (operations/msec)" << std::endl;

    for (int i = 0; i < x.size(); i++) {
        exp3_data << x[i] << "," << to_string(get_throughput[i]) << std::endl;
    }

    // Clear experiment db
    for (const auto &entry : std::filesystem::directory_iterator("./experiments_dbs"))
        std::filesystem::remove_all(entry.path());
}