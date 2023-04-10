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

// Generate experiments that measure the performance of your three operators, put, get and scan,
// as you insert more data into the system. The x-axis should report the data volume that had been inserted,
// while the y-axes should report throughput. Three figures should be produced, one for each operation.
void experiment1(int num_MB, int step_size_MB) {
    cout << "Running Experiment 1" << endl;

    DbOptions *options = new DbOptions();
    options->setSSTManager("BTree");
    options->setSSTSearch("BinarySearch");
    options->setBufferPoolType("None");

    SimpleKVStore db;
    db.open("./experiments_dbs/experiment_1", options);

    // Convert bytes to entries
    int num_inserts = num_MB * MEGABYTE / ENTRY_SIZE;
    int step_size = step_size_MB * MEGABYTE / ENTRY_SIZE;
    int num_queries = 0.00001 * num_inserts; // query 0.001% of data inserted

    std::cout << "Averaging from " + to_string(num_queries) + " queries" << std::endl;

    std::vector<int> x;
    std::vector<double> puts_throughput;
    std::vector<double> gets_throughput;
    std::vector<double> scans_throughput;

    int num_samples = 1.2 * num_inserts; // take a few extra samples to cover repeated puts
    int key_max = 50 * num_inserts; // add some randomness

    assert(key_max < INT_MAX);
    assert(step_size < num_inserts);

    std::cout << "Generating " + to_string(num_samples) + " random keys..." << std::endl;

    int rand_keys[num_samples];
    for (int j = 0; j < num_samples; j++) {
        rand_keys[j] = ::rand() % key_max; // not necessarily uniformly distributed to simulate real workload (skewed towards lower keys)
    }

    std::unordered_set<int> unique_keys;

    std::cout << "Generating " + to_string(num_inserts / step_size) + " datapoints..." << std::endl;
    std::cout << "Iteration: ";

    int offset = 0;
    for (int i = 0; i < num_inserts / step_size; i++) {

        std::cout << to_string(i + 1) << " ";
        fflush(stdout);

        // Check how many puts are needed to reach (i + 1) * step_size
        // unique keys in the db (might do some duplicate puts)
        // Do this computation here to prevent set operations from affecting
        // timed measurements
        int puts_count = 0;
        while (unique_keys.size() < (i + 1) * step_size) {
            int key = rand_keys[offset + puts_count];
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

    std::ofstream exp1_data ("./experiments/data/exp1_data.csv");

    exp1_data << "Inserted data (Bytes),Puts Throughput,Gets Throughput,Scans Throughput" << std::endl;

    for (int i = 0; i < x.size(); i++) {
        exp1_data << x[i] << "," << to_string(puts_throughput[i]) << "," << to_string(gets_throughput[i]) << "," << to_string(scans_throughput[i]) << std::endl;
    }

    db.close();

    // Clear experiment db
    for (const auto &entry : std::filesystem::directory_iterator("./experiments_dbs"))
        std::filesystem::remove_all(entry.path());
}
