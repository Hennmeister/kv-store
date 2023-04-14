#!/bin/bash



echo "=== Experiment 1 (running) ==="
./build/kv-store-performance-test -e 1 -d 256 -s 8 >> ./experiments/data/exp1.out
echo "Output saved to files ./experiments/data/ as exp1.out and exp1_data.csv"

echo "Experiment 2.1 (running)"
./build/kv-store-performance-test -e 21 -d 32 -s 1 > ./experiments/data/exp2p1.out
echo "Output saved to files ./experiments/data/ as exp2p1.out and exp2p1_data.csv"

echo "Experiment 2.2 (running)"
./build/kv-store-performance-test -e 22 -d 256 -s 8 > ./experiments/data/exp2p2.out
echo "Output saved to files ./experiments/data/ as exp2p2.out and exp2p2_data.csv"

echo "Experiment 3.2 (running)"
./build/kv-store-performance-test -e 32 -d 30 -s 1 > ./experiments/data/exp3p2.out
echo "Output saved to files ./experiments/data/ as exp3p2.out and exp3p2_data.csv"

echo "Experiment 3.1 (running)"
./build/kv-store-performance-test -e 31 -d 1024 -s 16 > ./experiments/data/exp3p1.out
echo "Output saved to files ./experiments/data/ as exp3p1.out and exp3p1_data.csv"







