#!/bin/bash

# Experiment 1 (inserting up to 1GB of data and plot with step size of 8 MB)
./build/kv-store-performance-test -e 1 -d 1024 -s 8

# Experiment 2.1 (cache size up to 128MB of data and plot with step size of 1 MB)
./build/kv-store-performance-test -e 21 -d 128 -s 1
# Experiment 2.2 (inserting up to 1GB of data and plot with step size of 8 MB)
./build/kv-store-performance-test -e 22 -d 1024 -s 8

# Experiment 3.1 (inserting up to 1GB of data and plot with step size of 8 MB)
./build/kv-store-performance-test -e 31 -d 1024 -s 8
# Experiment 3.2 (up to 128 bits per entry and plot with step size of 1)
./build/kv-store-performance-test -e 32 -d 128 -s 1


# Smaller experiments

#./build/kv-store-performance-test -e 1 -d 64 -s 4
#./build/kv-store-performance-test -e 21 -d 16 -s 1
#./build/kv-store-performance-test -e 22 -d 64 -s 4
#./build/kv-store-performance-test -e 32 -d 16 -s 1