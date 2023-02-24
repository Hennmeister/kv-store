# KV Store Report

## Introduction

## Design Decisions

## Performance Testing


### Experiment 1
`/kv-store-performance-test -e 1 -d MB_OF_DATA -s STEP_SIZE`

Experiment 1 aims to measure the throughput of the `put`, `get`, and `scan` operations. The current methodology is as follows:
1. Perform STEP_SIZE number of puts, where the key increments every iteration and the data is always 0, and measure the time taken
2. Generate random indices between 1 and the current key size
3. Perform STEP_SIZE number of gets on the random indices, and measure the time taken
4. Perform STEP_SIZE (-1) number of random scans over random, and measure the time
5. Graph the above times against the amount of data inserted into the kv-store at the time of measurement

The graphs are displayed and saved under the names `experiment1-{OPERATION}`.
Parameters:

    -e [num]: The experiment number to run
    -d [data amount]: The amount of data to run the experiement on in MB
    -s [num steps]: The number of operations to time (ex: 1000 means the time is measured every 1000 get operations)

Potential Issues:
- This is not an accurate measure of throughput. For this, we should do a multi-threaded approach. However, if the goal is simply to compare how our operations scale with the datastore size and compare our implementations, I think it's fine.
- How to best parameterize the experiment
- What keys/data we are inserting/querying for. For example, should we choose keys that are in the database for gets and scans? Would that help us gauge our performance better?