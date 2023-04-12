#include "kv-store-performance-testing.h"
#include <iostream>
#include "../include/util.h"
#include "experiment1.h"
#include "experiment2.h"
#include "experiment3.h"
#include <algorithm>
#include <filesystem>
#include <sys/stat.h>


using namespace std;

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

int main(int argc, char * argv[])
{
    cout << "Running Performance Testing..."<< endl;

    std::string path = "./experiments_dbs";

    if (dir_exists(path) == 0) {
        mkdir(path.c_str(), 0777);
    }
    else if (dir_exists(path) == 1)
    {
        for (const auto &entry : std::filesystem::directory_iterator(path))
            std::filesystem::remove_all(entry.path());
    }

    // Parse command line arguments
    int experiment_num = 1; // defaults to run all
    if (cmdOptionExists(argv, argv + argc, "-e")) {
        experiment_num = stoi(getCmdOption(argv, argv + argc, "-e"));
    }

    int num_MB = 32;
    if (cmdOptionExists(argv, argv + argc, "-d")) {
        num_MB = stoi(getCmdOption(argv, argv + argc, "-d"));
    }

    int step_size = 1;
    if (cmdOptionExists(argv, argv + argc, "-s")) {
        step_size = stoi(getCmdOption(argv, argv + argc, "-s"));
    }

    experiment_num = 21;
    num_MB = 8;
    step_size = 1;

    switch (experiment_num) {
        case 1: {
            experiment1(num_MB, step_size);
            break;
        }
        case 21: {
            experiment2p1(num_MB, step_size);
            break;
        }
        case 22: {
            experiment2p2(num_MB, step_size);
            break;
        }
        case 31: {
            experiment3p1(num_MB, step_size);
            break;
        }
        case 32: {
            // Repurpose num_MB
            int max_M = num_MB;
            experiment3p2(max_M, step_size);
            break;
        }
        default: {
            experiment3p1(num_MB, step_size);
        }
    }

    // Clear experiment db
    for (const auto &entry : std::filesystem::directory_iterator("./experiments_dbs"))
        std::filesystem::remove_all(entry.path());

    return 0;
}