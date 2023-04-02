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

// generate experiments that measure the performance of your three operators, put, get and scan,
// as you insert more data into the system. The x-axis should report the data volume that had been inserted,
// while the y-axes should report throughput. Three figures should be produced, one for each operation.
// These should be included in your report under the title “Experiments for Step 1”.
void experiment1(int num_MB, int step_size) {
    cout << "Running Experiment 1" << endl;
    SimpleKVStore db;
    db.open("experiment_1", PAGE_SIZE);

    // Multiply size of int by two since we are inserting both a key and a value
    long long int num_inserts = num_MB * MEGABYTE / (2 * sizeof(int));

    std::vector<long long int> x;
    std::vector<long long int> num_puts;
    std::vector<long long int> num_gets;
    std::vector<long long int> num_scans;
    std::vector<long long int> puts_microsecs;
    std::vector<long long int> gets_microsecs;
    std::vector<long long int> scans_microsecs;

    int num_samples = 1.2 * num_inserts; // take a few extra samples to cover repeated puts
    long long int key_max = 50 * num_inserts; // expect 2% of repeated puts

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

        x.push_back(unique_keys.size() * 2 * sizeof(int));
    }

    std::cout << std::endl;
    std::cout << "Printing to file..." << std::endl;

    std::ofstream exp1_data ("./experiments/data/exp1_data.txt");
    assert(x.size() == puts_microsecs.size());
    assert(puts_microsecs.size() == gets_microsecs.size());
    assert(scans_microsecs.size() == gets_microsecs.size());

    for (int i = 0; i < x.size(); i++) {
        exp1_data << x[i] << " " << to_string(num_puts[i]) << "," << to_string(puts_microsecs[i]) << " " << to_string(num_gets[i]) << "," << to_string(gets_microsecs[i]) << " " << to_string(num_scans[i]) << "," << to_string(scans_microsecs[i]) << std::endl;
    }

    // // display the plots
    // char y_label[80];
    // ::sprintf(y_label, "Throughput (operations/msec) - Averaged from %d samples", step_size);

    // plt::figure(1);
    // plt::plot(x, scans_microsecs);
    // plt::title("Experiment 1 - Scan Throughput");
    // plt::xlabel("Unique keys");
    // plt::ylabel(y_label);
    // plt::save("experiment1_scan");

    // plt::figure(2);
    // plt::plot(x, gets_microsecs);
    // plt::title("Experiment 1 - Get Throughput");
    // plt::xlabel("Unique keys");
    // plt::ylabel(y_label);
    // plt::save("experiment1_get");

    // plt::figure(3);
    // plt::plot(x, puts_microsecs);
    // plt::title("Experiment 1 - Put Throughput");
    // plt::xlabel("Unique keys");
    // plt::ylabel(y_label);
    // plt::save("experiment1_put");

    // plt::show();

    db.delete_data();
    db.close();
}

// Experiment 2.1: Compare the efficiency of LRU vs clock by measuring query throughput (on the y-axis) 
// as a function of the maximum buffer pool size (on the x-axis) by designing an appropriate experiment. 
// Try to identify one workload where LRU performs better and another workload where clock performs better. 


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

    switch (experiment_num) {
        case 1:
            experiment1(num_MB, step_size);
            break;
        default:
            experiment1(num_MB, step_size);
    }

    return 0;
}