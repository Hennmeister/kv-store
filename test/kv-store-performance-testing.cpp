//
// Created by Henning Lindig on 2023-02-20.
//

#include "kv-store-performance-testing.h"

#include <iostream>
#include <cassert>
#include "chrono"
#include "../include/SimpleKVStore.h"
#include "../include/RedBlackMemtable.h"
#include "../include/SimpleSSTManager.h"
#include "matplotlibcpp.h"
#include <algorithm>


using namespace std;
namespace plt = matplotlibcpp;

const long long MEGABYTE = 2 << 19;

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
    db.open("experiment_1",new RedBlackMemtable(), 30, new SimpleSSTManager( "experiment_1"));

    // Multiply size of int by two since we are inserting both a key and a value
    long long num_inserts = num_MB * MEGABYTE / (2 * sizeof(int));

    std::vector<long long > x;
    std::vector<long long> put_durations;
    std::vector<long long> get_durations;
    std::vector<long long> scan_durations;

    long long key = 0;
    for (int i = 0; i < num_inserts / step_size; i++) {
        auto start = chrono::high_resolution_clock::now();

        // run step_size number of puts
        for (int j = 0; j < step_size; j++) {
            db.put(key,0);
            key += 1;
        }
        auto stop = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(stop-start);
        put_durations.push_back(duration.count());

        // generate random indices
        long long rand_indices[step_size];
        for (int j = 0; j < step_size; j++) {
            rand_indices[j] = ::rand() % key;
        }

        // step_size number of random gets
        int val;
        start = chrono::high_resolution_clock::now();
        for (int j = 0; j < step_size; j++) {
            db.get(rand_indices[j], (int &) val);

        }
        stop = chrono::high_resolution_clock::now();
        duration = chrono::duration_cast<chrono::microseconds>(stop-start);
        get_durations.push_back(duration.count());

        // step_size number of random scans
        start = chrono::high_resolution_clock::now();
        for (int j = 1; j < step_size; j++) {
            db.scan(min(rand_indices[j-1], rand_indices[j]), max(rand_indices[j-1], rand_indices[j]));
        }
        stop = chrono::high_resolution_clock::now();
        duration = chrono::duration_cast<chrono::microseconds>(stop-start);
        scan_durations.push_back(duration.count());


        x.push_back(key * sizeof(int));
    }

    // display the plots
    char y_label[30];
    ::sprintf(y_label, "Time per %d operations (μs)", step_size);

    plt::figure(1);
    plt::plot(x, scan_durations);
    plt::title("Experiment 1 - Scan Throughput");
    plt::xlabel("data stored (Bytes)");
    plt::ylabel(y_label);
    plt::save("experiment1_scan");

    plt::figure(2);
    plt::plot(x, get_durations);
    plt::title("Experiment 1 - Get Throughput");
    plt::xlabel("data stored (Bytes)");
    plt::ylabel(y_label);
    plt::save("experiment1_get");

    plt::figure(3);
    plt::plot(x, put_durations);
    plt::title("Experiment 1 - Put Throughput");
    plt::xlabel("data stored (Bytes)");
    plt::ylabel(y_label);
    plt::save("experiment1_put");

    plt::show();

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