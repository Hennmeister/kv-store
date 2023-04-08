#!/bin/bash

# Experiment 1 (inserting up to 1GB of data and plot step size of 8 MB)
./build/kv-store-performance-test -e 1 -d 1024 -s 8

# Experiment 2.1 (Cache size up to 10MB of data and plot step size of 1 KB)
./build/kv-store-performance-test -e 21 -d 10 -s 
# Experiment 2.2
./build/kv-store-performance-test -e 22 -d 1024 -s 1

# Experiment 3.1
./build/kv-store-performance-test -e 31 -d 1024 -s 1
# Experiment 3.2 
./build/kv-store-performance-test -e 32 -d 1024 -s 1