#include <iostream>
#include <cassert>
#include "chrono"
#include "../include/SimpleKVStore.h"
#include "../include/util.h"
#include <algorithm>
#include <fstream>

using namespace std;

// Generate experiments that measure the performance of your three operators, put, get and scan,
// as you insert more data into the system. The x-axis should report the data volume that had been inserted,
// while the y-axes should report throughput. Three figures should be produced, one for each operation.
void experiment1(int num_MB, int step_size_MB) {
    cout << "Running Experiment 1" << endl;

    DbOptions *options = new DbOptions();
    options->setSSTManager("BTreeManager");
    options->setSSTSearch("BinarySearch");
    options->setBufferPoolType("None");
    options->setFilterBitsPerEntry(0);

    SimpleKVStore db;
    db.open("./experiments_dbs/experiment_1", options);

    // Convert bytes to entries
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

        int num_queries = 0.00001 * db_num_keys; // query 0.001% of data inserted

        // num_queries number of random gets
        int val;
        start = chrono::high_resolution_clock::now();
        for (int j = 0; j < num_queries; j++)
            db.get(::rand() % db_num_keys, (int &) val);

        stop = chrono::high_resolution_clock::now();
        microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();
        gets_throughput.push_back((1000 * (double)num_queries) / microsecs);

        // 0.5 * num_queries number of random scans
        start = chrono::high_resolution_clock::now();
        for (int j = 1; j < 0.5 * num_queries; j++) {
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

    std::ofstream exp1_data ("./experiments/data/exp1_data.csv");

    exp1_data << "Inserted data (Bytes),Puts Throughput,Gets Throughput,Scans Throughput" << std::endl;

    for (int i = 0; i < x.size(); i++) {
        exp1_data << x[i] << "," << to_string(puts_throughput[i]) << "," << to_string(gets_throughput[i]) << "," << to_string(scans_throughput[i]) << std::endl;
    }

    db.close();
}
