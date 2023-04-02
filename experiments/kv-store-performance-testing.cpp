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

    std::vector<double> x;
    std::vector<double> num_puts;
    std::vector<double> num_gets;
    std::vector<double> num_scans;
    std::vector<double> puts_msecs;
    std::vector<double> gets_msecs;
    std::vector<double> scans_msecs;

    // Add more keys for increased randomness and guaranteed enough unique values
    long long int key_max = 2.0 * num_inserts;

    long long int rand_keys[key_max];
    for (int j = 0; j < key_max; j++) {
        rand_keys[j] = ::rand() % key_max;
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
            unique_keys.insert(rand_keys[offset + puts_count]);
            puts_count++;
        }

        auto start = chrono::high_resolution_clock::now();

        // actually run and time correct number of puts
        for (int j = 0; j < puts_count; j++) {
            db.put(rand_keys[offset + j], 0); // paylod is irrelevant
        }
        auto stop = chrono::high_resolution_clock::now();
        double msecs = chrono::duration_cast<chrono::microseconds>(stop-start).count() / 1000;
        puts_msecs.push_back(msecs);
        num_puts.push_back(puts_count);

        offset = puts_count;

        // generate step_size random keys from the unique inserted keys
        long long int rand_gets[step_size];
        for (int j = 0; j < step_size; j++) {
            long long key;
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
        msecs = chrono::duration_cast<chrono::microseconds>(stop-start).count() / 1000;
        gets_msecs.push_back(msecs);
        num_gets.push_back(step_size);

        // step_size number of random scans
        start = chrono::high_resolution_clock::now();
        for (int j = 1; j < step_size; j++)
            db.scan(min(rand_gets[j-1], rand_gets[j]), max(rand_gets[j-1], rand_gets[j]));

        stop = chrono::high_resolution_clock::now();
        msecs = chrono::duration_cast<chrono::microseconds>(stop-start).count() / 1000;
        scans_msecs.push_back(msecs);
        num_scans.push_back(step_size);

        x.push_back(unique_keys.size());
    }

    std::cout << std::endl;
    std::cout << "Printing to file..." << std::endl;

    std::ofstream exp1_data ("./experiments/data/exp1_data.txt");
    assert(x.size() == puts_msecs.size());
    assert(puts_msecs.size() == gets_msecs.size());
    assert(scans_msecs.size() == gets_msecs.size());

    for (int i = 0; i < x.size(); i++) {
        exp1_data << x[i] << "," << to_string(puts_msecs[i]) << "-" << to_string(num_puts[i]) << "," << to_string(gets_msecs[i]) << "-" << to_string(num_gets[i]) << "," << to_string(scans_msecs[i]) << "-" << to_string(num_scans[i]) << std::endl;
    }

    // // display the plots
    // char y_label[80];
    // ::sprintf(y_label, "Throughput (operations/msec) - Averaged from %d samples", step_size);

    // plt::figure(1);
    // plt::plot(x, scans_msecs);
    // plt::title("Experiment 1 - Scan Throughput");
    // plt::xlabel("Unique keys");
    // plt::ylabel(y_label);
    // plt::save("experiment1_scan");

    // plt::figure(2);
    // plt::plot(x, gets_msecs);
    // plt::title("Experiment 1 - Get Throughput");
    // plt::xlabel("Unique keys");
    // plt::ylabel(y_label);
    // plt::save("experiment1_get");

    // plt::figure(3);
    // plt::plot(x, puts_msecs);
    // plt::title("Experiment 1 - Put Throughput");
    // plt::xlabel("Unique keys");
    // plt::ylabel(y_label);
    // plt::save("experiment1_put");

    // plt::show();

    db.delete_data();
    db.close();
}

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