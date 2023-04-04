#include "kv-store-performance-testing.h"

#include <iostream>
#include <cassert>
#include "chrono"
#include "../include/SimpleKVStore.h"
#include "../include/RedBlackMemtable.h"
#include "../include/SimpleSSTManager.h"
#include "../include/constants.h"
#include <algorithm>
#include <unordered_set>
#include <random>
#include <iostream>
#include <fstream>

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
// These should be included in your report under the title “Experiments for Step 1”.
void experiment1(int num_MB, int step_size) {
    cout << "Running Experiment 1" << endl;
    SimpleKVStore db;
    db.open("experiment_1", PAGE_SIZE);

    // Multiply size of int by two since we are inserting both a key and a value
    long long int num_inserts = num_MB * MEGABYTE / ENTRY_SIZE;

    std::vector<long long int> x;
    std::vector<long long int> num_puts;
    std::vector<long long int> num_gets;
    std::vector<long long int> num_scans;
    std::vector<long long int> puts_microsecs;
    std::vector<long long int> gets_microsecs;
    std::vector<long long int> scans_microsecs;

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
        int microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();
        puts_microsecs.push_back(microsecs);
        num_puts.push_back(puts_count);

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
        gets_microsecs.push_back(microsecs);
        num_gets.push_back(step_size);

        // step_size number of random scans
        start = chrono::high_resolution_clock::now();
        for (int j = 1; j < step_size; j++)
            db.scan(min(rand_gets[j-1], rand_gets[j]), max(rand_gets[j-1], rand_gets[j]));

        stop = chrono::high_resolution_clock::now();
        microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();
        scans_microsecs.push_back(microsecs);
        num_scans.push_back(step_size);

        x.push_back(unique_keys.size() * ENTRY_SIZE);
    }

    std::cout << std::endl;
    std::cout << "Printing to file..." << std::endl;

    assert(x.size() == puts_microsecs.size());
    assert(puts_microsecs.size() == gets_microsecs.size());
    assert(scans_microsecs.size() == gets_microsecs.size());
    
    std::ofstream exp1_data ("./experiments/data/exp1_data.csv");

    exp1_data << "Inserted data (Bytes),Puts Count,Puts time (microsecs),Gets Count,Gets time (microsecs),Scans Count,Scans time (microsecs)" << std::endl;

    for (int i = 0; i < x.size(); i++) {
        exp1_data << x[i] << "," << to_string(num_puts[i]) << "," << to_string(puts_microsecs[i]) << "," << to_string(num_gets[i]) << "," << to_string(gets_microsecs[i]) << "," << to_string(num_scans[i]) << "," << to_string(scans_microsecs[i]) << std::endl;
    }

    db.close();
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
    SimpleKVStore lru_db1;
    SimpleKVStore clock_db1;
    SimpleKVStore lru_db2;
    SimpleKVStore clock_db2;
    lru_db1.open("lru_db", PAGE_SIZE); // TODO make lru only and no LSM tree
    clock_db1.open("clock_db", PAGE_SIZE); // TODO make clock only and no LSM tree
    lru_db2.open("lru_db", PAGE_SIZE); // TODO make lru only and no LSM tree
    clock_db2.open("clock_db", PAGE_SIZE); // TODO make clock only and no LSM tree

    long long int max_buf_size = num_MB * MEGABYTE;

    // Load 100 * max_buf_size bytes of entries in the database
    long long int num_inserts = 100 * max_buf_size / ENTRY_SIZE;
    long long int num_queries = num_inserts;

    std::vector<long long int> x;
    std::vector<long long int> lru_throughput1;
    std::vector<long long int> clock_throughput1;
    std::vector<long long int> lru_throughput2;
    std::vector<long long int> clock_thrlru_throughput2;

    assert(step_size < num_inserts);

    std::vector<long long int> rand_keys(num_inserts);
    std::iota(rand_keys.begin(), rand_keys.end(), 1); // fills vector with increasing keys
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(rand_keys.begin(), rand_keys.end(), gen);

    // Load dbs with randomly ordered keys
    for (int i = 0; i < rand_keys.size(); i++) {
        clock_db1.put(rand_keys[i], 0);
        lru_db1.put(rand_keys[i], 0);
        clock_db2.put(i, 0);
        lru_db2.put(i, 0);
    }

    std::cout << "Generating " + to_string(max_buf_size / step_size) + " datapoints..." << std::endl;
    std::cout << "Iteration: ";

    long long int offset = 0;
    for (int i = 0; i < max_buf_size / step_size; i++) {
        int buffer_size = (i + 1) * step_size;

        // TODO: set buffer size here to buffer_size
        
        // 1. === CLOCK BETTER ===

        int val;

        // Time LRU
        auto start = chrono::high_resolution_clock::now();
        
        for (int i = 0; i < num_queries; i++) 
            lru_db1.get(rand_int(1, num_inserts), (int &) val);

        auto stop = chrono::high_resolution_clock::now();
        double microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();

        lru_throughput1.push_back(((double) num_queries) / microsecs);

        // Time Clock
        auto start = chrono::high_resolution_clock::now();
        
        for (int i = 0; i < num_queries; i++) 
            clock_db1.get(rand_int(1, num_inserts), (int &) val);

        auto stop = chrono::high_resolution_clock::now();
        double microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();

        clock_throughput1.push_back(((double) num_queries) / microsecs);
        
        // 2. === LRU BETTER ===

        // Set key range we are iterating over

        int num_pages_in_buf = buffer_size / PAGE_SIZE;

        // Reaccess the same page after querying 30% of the buffer size new pages
        // This tries to be long enough so that clock handle did not tick
        int reaccess_after = 0.30 * num_pages_in_buf; // take the floor of 30%

        // Get keys PAGE_NUM_ENTRIES apart to gurantee different entries
        int start_key = rand_int(1, num_entries - (reaccess_after * PAGE_NUM_ENTRIES));

        std::vector<long long int> keys_iterated = {start_key};
        for (int i = 0; i < reaccess_after; i++)
            keys_inserted.push_back(start_key + i * PAGE_NUM_ENTRIES);

        // Time LRU
        start = chrono::high_resolution_clock::now();

        for (int i = 0; i < num_queries; i++) {
            // To avoid synchronizing with the clock handle, query from
            // random key after 10% of buffer size queries happened
            if (i % (0.1 * num_page_buf) == 0) {
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
            if (i % (0.1 * num_page_buf) == 0) {
                clock_db2.get(rand_int(1, num_inserts), (int &) val);
            } else {
                clock_db2.get(keys_iterated[i % reaccess_after + 1], (int &) val);
            }
        }

        stop = chrono::high_resolution_clock::now();
        microsecs = chrono::duration_cast<chrono::microseconds>(stop-start).count();

        clock_throughput2.push_back(((double) num_queries) / microsecs);

        x.push_back((i + 1) * ENTRY_SIZE);
    } 

    std::cout << std::endl;
    std::cout << "Printing to file..." << std::endl;

    assert(x.size() == lru_throughput1.size());
    assert(lru_throughput1.size() == clock_throughput1.size());
    assert(clock_throughput1.size() == lru_throughput2.size());
    assert(lru_throughput2.size() == clock_throughput2.size());
    
    std::ofstream exp2_data ("./experiments/data/exp2_data.csv");

    exp1_data << "Max Cache Size (Bytes),LRU Throughput (Clock better),Clock Throughput (Clock better),LRU Throughput (LRU better),Clock Throughput (LRU better)" << std::endl;

    for (int i = 0; i < x.size(); i++) {
        exp1_data << x[i] << "," << to_string(lru_throughput1[i]) << "," << to_string(clock_throughput1[i]) << "," << to_string(lru_throughput2[i]) << "," << to_string(clock_throughput2[i]) << std::endl;
    }

    db.close();
}

// Experiment 2.2: Design an experiment comparing your binary search to B-tree search in terms of query 
// throughput (on the y-axis) as you increase the data size (on the x-axis). 
// This experiment should be done with uniformly randomly distributed queries and data.



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

    // TODO: reproducibility
    switch (experiment_num) {
        case 1:
            experiment1(num_MB, step_size);
            break;
        default:
            experiment1(num_MB, step_size);
    }

    return 0;
}