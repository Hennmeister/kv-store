#include "kv-store-performance-testing.h"

#include <iostream>
#include <cassert>
#include "chrono"
#include "../include/SimpleKVStore.h"
#include "../include/constants.h"
#include "../include/Base/DbOptions.h"
#include <algorithm>
#include <unordered_set>
#include <random>
#include <iostream>
#include <fstream>
#include <filesystem>

#define AVERAGE_NUM 5000

using namespace std;

const long long int MEGABYTE = 2 << 19;

// https://stackoverflow.com/questions/865668/parsing-command-line-arguments-in-c
char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
    return std::find(begin, end, option) != end;
}

int rand_int(int range_from, int range_to) {
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<int> distr(range_from, range_to);
    return distr(generator);
}

// Generate experiments that measure the performance of your three operators, put, get and scan,
// as you insert more data into the system. The x-axis should report the data volume that had been inserted,
// while the y-axes should report throughput. Three figures should be produced, one for each operation.
void experiment1(int num_MB, int step_size_MB) {
    cout << "Running Experiment 1" << endl;

    DbOptions *options = new DbOptions();
    options->setSSTSearch("BinarySearch");
    options->setBufferPoolType("None");

    SimpleKVStore db;
    db.open("./experiments_dbs/experiment_1", options);

    // Convert bytes to entries
    long long int num_inserts = num_MB * MEGABYTE / ENTRY_SIZE;
    long long int step_size = step_size_MB * MEGABYTE / ENTRY_SIZE;

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
    
    std::ofstream exp1_data ("./experiments/data/exp1_data.csv");

    exp1_data << "Inserted data (Bytes),Puts Throughput (operations/msec),Gets Throughput (operations/msec),Scans Throughput (operations/msec)" << std::endl;

    for (int i = 0; i < x.size(); i++) {
        exp1_data << x[i] << "," << to_string(puts_throughput[i]) << "," << to_string(gets_throughput[i]) << "," << to_string(scans_throughput[i]) << std::endl;
    }

    db.close();

    // Clear experiment db
    for (const auto &entry : std::filesystem::directory_iterator("./experiments_dbs"))
        std::filesystem::remove_all(entry.path());
}

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

    long long int max_buf_size = num_MB * MEGABYTE;

    // Load 100 * max_buf_size bytes of entries in the database
    long long int num_inserts = 100 * max_buf_size / ENTRY_SIZE;
    long long int num_queries = num_inserts * ((double) step_size / (double) max_buf_size);

    std::cout << "Averaging " + to_string(num_queries) + " queries" << std::endl;

    std::vector<long long int> x;
    std::vector<long long int> lru_throughput1;
    std::vector<long long int> clock_throughput1;
    std::vector<long long int> lru_throughput2;
    std::vector<long long int> clock_throughput2;

    assert(step_size < num_inserts);

    std::vector<long long int> rand_keys(num_inserts);

    // fills vector with increasing keys starting from 1
    std::iota(rand_keys.begin(), rand_keys.end(), 1); 
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(rand_keys.begin(), rand_keys.end(), gen);

    for (int i = 0; i < rand_keys.size(); i++) {
        // Load dbs with randomly ordered keys
        clock_db1.put(rand_keys[i], 0);
        lru_db1.put(rand_keys[i], 0);

        // Load dbs with sequential keys
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

        clock_throughput1.push_back(((double) num_queries) / microsecs);
        
        // 2. === LRU BETTER ===

        // Set key range we are iterating over

        int num_pages_in_buf = buffer_size / PAGE_SIZE;

        // Reaccess the same page after querying 30% of the buffer size new pages
        // This tries to be long enough so that clock handle did not tick
        int reaccess_after = 0.30 * num_pages_in_buf; // take the floor of 30%

        // Get keys PAGE_NUM_ENTRIES apart to gurantee different entries
        int start_key = rand_int(1, num_inserts - (reaccess_after * PAGE_NUM_ENTRIES));

        std::vector<long long int> keys_iterated = {start_key};
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

        lru_throughput2.push_back(((double) num_queries) / microsecs);

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

        clock_throughput2.push_back(((double) num_queries) / microsecs);

        x.push_back((i + 1) * step_size * ENTRY_SIZE);
    } 

    std::cout << std::endl;
    std::cout << "Printing to file..." << std::endl;

    assert(x.size() == lru_throughput1.size());
    assert(lru_throughput1.size() == clock_throughput1.size());
    assert(clock_throughput1.size() == lru_throughput2.size());
    assert(lru_throughput2.size() == clock_throughput2.size());
    
    std::ofstream exp2_data ("./experiments/data/exp2p1_data.csv");
    
    exp2_data << "Max Cache Size (Bytes),LRU Throughput - Clock better (operations/msec),Clock Throughput - Clock better (operations/msec),LRU Throughput - LRU better (operations/msec),Clock Throughput - LRU better (operations/msec)" << std::endl;

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
void experiment2p2(int num_MB, int step_size) {
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
    long long int num_inserts = num_MB * MEGABYTE / ENTRY_SIZE;
    long long int num_queries = 0.001 * num_inserts;

    std::vector<long long int> x;
    std::vector<long long int> btree_throughput;
    std::vector<long long int> bs_throughput;

    assert(step_size < num_inserts);

    std::vector<long long int> rand_keys(num_inserts);

    // fills vector with increasing keys starting from 1
    std::iota(rand_keys.begin(), rand_keys.end(), 1); 
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(rand_keys.begin(), rand_keys.end(), gen);

    // Load dbs with randomly ordered keys
    for (int i = 0; i < rand_keys.size(); i++) {
        btree_db.put(rand_keys[i], 0);
        bs_db.put(rand_keys[i], 0);
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

        btree_throughput.push_back(((double) num_queries) / microsecs);

        // Time and query BTree keys uniformly at random
        start = chrono::high_resolution_clock::now();
        
        for (int i = 0; i < num_queries; i++) 
            bs_db.get(rand_int(1, num_inserts), (int &) val);

        stop = chrono::high_resolution_clock::now();
        microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();

        bs_throughput.push_back(((double) num_queries) / microsecs);
        
        x.push_back((i + 1) * step_size * ENTRY_SIZE);
    } 

    std::cout << std::endl;
    std::cout << "Printing to file..." << std::endl;

    assert(x.size() == btree_throughput.size());
    assert(btree_throughput.size() == bs_throughput.size());
    
    std::ofstream exp2_data ("./experiments/data/exp2p2_data.csv");

    exp2_data << "Inserted data (Bytes),BTree Search Throughput (operations/msec),Binary Search Throughput (operations/msec)" << std::endl;

    for (int i = 0; i < x.size(); i++) {
        exp2_data << x[i] << "," << to_string(btree_throughput[i]) << "," << to_string(bs_throughput[i]) << std::endl;
    }

    btree_db.close();
    bs_db.close();

    // Clear experiment db
    for (const auto &entry : std::filesystem::directory_iterator("./experiments_dbs"))
        std::filesystem::remove_all(entry.path());
}

// // Experiment 3.1: Measure insertion, get, and scan throughput for your implementation over time as the data size 
// // grows. Describe your experimental setup and make sure all relevant variables are controlled. Please fix the buffer 
// // pool size to 10 MB, the Bloom filters to use 5 bits per entry, and the memtable to 1 MB. Run this experiment as you 
// // insert 1 GB of data. Measure get and scan throughput at regular intervals as you insert this data.
// void experiment3p1(int num_MB, int step_size) {
//     cout << "Running Experiment 3.1" << endl;

//     SimpleKVStore db;
//     db.open("./experiments_dbs/experiment_3p1"); // TODO: specs

//     // Load 1 GB of data
//     long long int num_inserts = 1024 * MEGABYTE / ENTRY_SIZE;

//     std::vector<long long int> x;
//     std::vector<long long int> puts_throughput;
//     std::vector<long long int> gets_throughput;
//     std::vector<long long int> scans_throughput;

//     int num_samples = 1.2 * num_inserts; // take a few extra samples to cover repeated puts
//     long long int key_max = 50 * num_inserts; // add some randomness 

//     assert(key_max < INT_MAX);
//     assert(step_size < num_inserts);

//     long long int rand_keys[num_samples];
//     for (int j = 0; j < num_samples; j++) {
//         rand_keys[j] = ::rand() % key_max; // not necessarily uniformly distributed to simulate real workload (skewed towards lower keys)
//     }

//     std::unordered_set<long long int> unique_keys;

//     std::cout << "Generating " + to_string(num_inserts / step_size) + " datapoints..." << std::endl;
//     std::cout << "Iteration: ";

//     long long int offset = 0;
//     for (int i = 0; i < num_inserts / step_size; i++) {

//         std::cout << to_string(i + 1) << " ";
//         fflush(stdout);

//         // Check how many puts are needed to reach (i + 1) * step_size 
//         // unique keys in the db (might do some duplicate puts)
//         // Do this computation here to prevent set operations from affecting
//         // timed measurements 
//         int puts_count = 0;
//         while (unique_keys.size() < (i + 1) * step_size) {
//             long long int key = rand_keys[offset + puts_count];
//             if (unique_keys.find(key) == unique_keys.end()) {
//                 unique_keys.insert(key);
//             } 
//             puts_count++;
//         }

//         auto start = chrono::high_resolution_clock::now();

//         // actually run and time correct number of puts
//         for (int j = 0; j < puts_count; j++) {
//             db.put(rand_keys[offset + j], 0); // paylod is irrelevant
//         }
//         auto stop = chrono::high_resolution_clock::now();
//         double microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();
//         puts_throughput.push_back((1000 * (double)puts_count) / microsecs);

//         offset += puts_count;

//         // generate step_size random keys from the unique inserted keys
//         long long int rand_gets[step_size];
//         for (int j = 0; j < step_size; j++) {
//             long long int key;
//             std::mt19937 gen(std::random_device{}());
//             std::sample(unique_keys.begin(), unique_keys.end(), &key, 1, gen);
//             rand_gets[j] = key;
//         }

//         // step_size number of random gets
//         int val;
//         start = chrono::high_resolution_clock::now();
//         for (int j = 0; j < step_size; j++) 
//             db.get(rand_gets[j], (int &) val);

//         stop = chrono::high_resolution_clock::now();
//         microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();
//         gets_throughput.push_back((1000 * (double)step_size) / microsecs);

//         // step_size number of random scans
//         start = chrono::high_resolution_clock::now();
//         for (int j = 1; j < step_size; j++)
//             db.scan(min(rand_gets[j-1], rand_gets[j]), max(rand_gets[j-1], rand_gets[j]));

//         stop = chrono::high_resolution_clock::now();
//         microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();
//         scans_throughput.push_back((1000 * (double)step_size) / microsecs);

//         x.push_back(unique_keys.size() * ENTRY_SIZE);
//     }

//     std::cout << std::endl;
//     std::cout << "Printing to file..." << std::endl;

//     assert(x.size() == puts_throughput.size());
//     assert(puts_throughput.size() == gets_throughput.size());
//     assert(gets_throughput.size() == scans_throughput.size());
    
//     std::ofstream exp3_data ("./experiments/data/exp3p1_data.csv");

//     exp3_data << "Inserted data (Bytes),Puts Throughput (operations/msec),Gets Throughput (operations/msec),Scans Throughput (operations/msec)" << std::endl;

//     for (int i = 0; i < x.size(); i++) {
//         exp3_data << x[i] << "," << to_string(puts_throughput[i]) << "," << to_string(gets_throughput[i]) << "," << to_string(scans_throughput[i]) << std::endl;
//     }

//     db.close();

//     // Clear experiment db
//     for (const auto &entry : std::filesystem::directory_iterator("./experiments_dbs"))
//         std::filesystem::remove_all(entry.path());
// }

 
// // Experiment 3.2: Illustrate an experiment showing get performance as you vary the number of bits per entry used for 
// // your Bloom filters. 
// void experiment3p2(int max_M, int step_size) {
//     cout << "Running Experiment 3.2" << endl;

//     // Load 1 GB of data on each run
//     long long int num_inserts = 1024 * MEGABYTE / ENTRY_SIZE;
//     long long int num_queries = 0.01 * num_inserts; // query 1% of db size

//     std::vector<long long int> x;
//     std::vector<long long int> get_throughput;

//     assert(step_size < num_inserts);

//     std::vector<long long int> rand_keys(num_inserts);

//     // fills vector with increasing keys starting from 1
//     std::iota(rand_keys.begin(), rand_keys.end(), 1); 
    
//     std::random_device rd;
//     std::mt19937 gen(rd());

//     // Generate shuffled insert order
//     std::shuffle(rand_keys.begin(), rand_keys.end(), gen);

//     std::cout << "Generating " + to_string(max_M / step_size) + " datapoints..." << std::endl;
//     std::cout << "Iteration: ";

//     for (int i = 0; i < max_M / step_size; i++) {
//         int M = (i + 1) * step_size;
//         std::cout << to_string(i + 1) << " ";
//         fflush(stdout);

//         // TODO: set bits per entry here

//         SimpleKVStore db;
//         db.open("./experiments_dbs/exp3p2_" + to_string(M) + "bits_per_entry");

//         // Load db with randomly ordered keys
//         for (int i = 0; i < rand_keys.size(); i++)
//             db.put(rand_keys[i], 0);

//         // Time queries
//         int val;
//         auto start = chrono::high_resolution_clock::now();

//         for (int i = 0; i < num_queries; i++) 
//             db.get(rand_int(1, num_queries), (int &) val);

//         auto stop = chrono::high_resolution_clock::now();
//         double microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();

//         get_throughput.push_back((1000 * (double) num_queries) / microsecs);

//         x.push_back(M);
//     }

//     std::cout << std::endl;
//     std::cout << "Printing to file..." << std::endl;

//     assert(x.size() == get_throughput.size());
    
//     std::ofstream exp3_data ("./experiments/data/exp3p2_data.csv");
    
//     exp3_data << "Bloom filter bits per entry (M),Get Throughput (operations/msec)" << std::endl;

//     for (int i = 0; i < x.size(); i++) {
//         exp3_data << x[i] << "," << to_string(get_throughput[i]) << std::endl;
//     }

//     db.close();

//     // Clear experiment db
//     for (const auto &entry : std::filesystem::directory_iterator("./experiments_dbs"))
//         std::filesystem::remove_all(entry.path());
// }

int main(int argc, char * argv[])
{
    cout << "Running Performance Testing..."<< endl;

    // Parse command line arguments
    int experiment_num = 0; // defaults to run all
    if (cmdOptionExists(argv, argv + argc, "-e")) {
        experiment_num = stoi(getCmdOption(argv, argv + argc, "-e"));
    }

    int num_MB = 1;
    if (cmdOptionExists(argv, argv + argc, "-d")) {
        num_MB = stoi(getCmdOption(argv, argv + argc, "-d"));
    }

    int step_size = 1000;
    if (cmdOptionExists(argv, argv + argc, "-s")) {
        step_size = stoi(getCmdOption(argv, argv + argc, "-s"));
    }

    switch (experiment_num) {
        case 1:
            experiment1(num_MB, step_size);
            break;
        case 21:
            experiment2p1(num_MB, step_size);
            break;
        case 22:
            experiment2p2(num_MB, step_size);
            break;
        // case 31:
        //     // num_MB useless in this experiment
        //     experiment3p1(0, step_size);
        //     break;
        // case 32:
        //     // Repurpose num_MB
        //     int max_M = num_MB;
        //     experiment3p2(max_M, step_size);
        //     break;
        default:
            experiment1(num_MB, step_size);
    }

    return 0;
}