# KV Store Report

Refer to [this page](https://docs.google.com/document/d/1dsIuIzXiIBbiZcNYi1cC62PVE4mF-MdksmIQ1hikFGM) for the official project handout.

## Introduction

In this project, we build a key-value store from scratch. A key-value store (KV-store) is a kind of database system that stores key-value pairs and allows retrieval of a value based on its key. KV-stores exhibit the following simple API:

- `Open(“database name”)` opens your database and prepares it to run
- `Put(key, value)` stores a key associated with a value
- `Value = Get(key)` retrieves a value associated with a given key
- `KV-pairs = Scan(Key1, Key2)` retrieves all KV-pairs in a key range in key order (key1 < key2)
- `Close()` closes your database

This API also reflects common data structures (e.g., binary trees). Unlike simple binary trees, however, a KV-store must be able to store data greater than the amount of memory your system has available. In other words, it has to spill data to storage (disk or SSD) as the size of the data grows.

We implement this key-value store using various data structures covered in class. Specifically, we implement an LSM-tree in storage complemented by in-memory Bloom filters. This architecture follows that of many popular modern KV-stores, including RocksDB, Cassandra, HBase, etc.

KV-stores are widely used in industry. Note that they have a far simpler API than traditional relational database systems, which expose SQL as an API to the user. There are many applications for which a simple KV API is sufficient. Note, however, that KV-stores can also be used as the backbone for relational database management systems. For example, MyRocks by Meta is an example of a relational database utilizing as its backbone a key-value store very similar to the one we built, namely RocksDB.

## "I don't care, just tell me how to run it." - Fine...

Run `./make.sh` to compile the program and run the tests in one command. The test executable file will be located under `/build/kv-store-test`. It also compiles the experiments executable at `/build/kv-store-performance-test` and a playground executable at `/build/kv-store-performance-test`.

### Playground

We provide an empty C++ file with a default version of our database (that you are free modify if you wish - see [Database Initialization and Parameters](#database-initialization-and-parameters)) for you to play with and write any calls to `open`, `put`, `get`, `scan`, print results and verify functionality. The aim is to provide you with an easy way to interact with our database without having to compile a new project.

### Experiments

Run `./experiments.sh` to run the excutable that generates all experiments data. You can also generate data individually for each experiment by calling the executable file `/build/kv-store-performance-test` with the parameters indicated on the calls of `experiments.sh`. We also provide a `plot_experiments.sh` script that plots the data generated for each experiment. You can also plot the data of individual experiments by using the same approach.

## Project Status

TODO: at the end

## Database Initialization and Parameters

We provide the user with a DbOptions object that is used to set default configurations for any database instantiated, and also giving the freedom to specify some options based on the user's preference.

As such, open("database name") is a valid call to create a database if the user simply wants a functioning databse without thinking about possible parameters/options, but they may also generate a DbOptions object and pass that in as a second parameter to the call for further customization.

### DbOptions

Default values: TODO

Options: TODO

An example is as follows:

````md
  example
  example
  example
````

## Implementation Steps

Here we outline the process of implementation the various parts of our system. For simplicity, our simple KV-Store only handles integer keys and integer values.

The general flow is the following: entries get populated in a memtable (fitting entirely in memory) that holds the most recent key-value insertions in the database. Once the memtable grows beyond its capacity, the contents of the memtable are dumped to an SST sorted

### Abtractions

Aware of the possible changes to the algorithms as well as our methodology, either as a consequence of efficiency tradeoffs or improvements to the system, we tried to structure our OOP code in a way that maximizes the use of abstractions/interfaces while minimizing the amount of coupling our classes have between each other.

TODO: @VijayS02 feel free to elaborate if needed

### Step 1

- **Memtable**

We implement a memtable as a balanced binary search tree ([red-black tree](https://en.wikipedia.org/wiki/Red%E2%80%93black_tree) to be precise) supporting standard `put(key,val)`, `get(key)` and `scan(key1, key2)` methods. We also implement an inorder tree traversal that returns all the key-value pairs sorted by their keys, which will be helpful to dump the content into the corresponding SST (refer to next bullet point).

- **SSTs (Sorted String Tables)**

We set a maximum capacity (e.g. a page size of 4KB) to the Memtable, at which point we dump the contents key-value pairs in sorted order to an SST file `sstX`. The SSTs are thus stored in decreasing order of longevity where `sst1` is the oldest Memtable dumped. On a get query that is not found on the current Memtable, our database traverses over the SSTs from newest to oldest to find a key. Note that we implement the SST dump so that it writes in binary so an append-only file to maximize efficiency in sequential writes.

#### Experiments

TODO: step1 experiments

### Step 2

- **Buffer Pool**

TODO

- **Eviction Policies**

TODO

- **B-Tree for SST**

TODO

#### Experiments

TODO: step2 experiments

### Step 3

- **LSM Tree**

TODO

- **Incorporating Updates and Deletes**

TODO

- **Bloom Filter**

TODO

#### Experiments

TODO: step3 experiments

## Testing

TODO


Marking scheme:

Marking scheme (tentative) - Total 100 points + bonus (10 points)
Experiments - 33 points
Core Implementation - 52 points
Software Engineering practices - 15 points

List of report must-haves

-Description of design elements - concisely describe the different elements of your design (i.e., memtable, B-tree creation, extendible hashing, etc), and any interesting design decision you have made. This should correspond to the list of design elements below. Feel free to tell us about extra cool stuff you might have added. Please also tell us where to find the implementation of each design element in the code (i.e., file and function name). 
-Project status - Give details on what works and what does not. If there are known bugs in your code, list them here. 
-Experiments - please show experimental figures and explain your findings for each experiment. If you have some extra experiments that you did, put them here under a separate “Extra Experiments” heading.
-Testing - Testing is a very important part of any implementation, Mention how you tested your implementation, if you have several unit and integration tests, list them here.
-Compilation & running Instructions - detail out how to run your project (i.e., give makefile targets and describe what they are for). If you have an executable that we can use to run simple commands, give instructions on how to use that.

List of coding practices to check

-Code readability - consistent naming and indentation, meaningful identifiers, comments, no too long lines (3 points)
-Code modularity - short and specific task functions, good code reuse (3 points)
-Version control - use of version control to collaborate and making meaningful commits (2 points)
-Makefile - makefile for experiments and executable build (doesn’t have to be separate as long as instructions are in the report) - (2 points)
-Tests - correctness and performance tests (5 points)

List of Design Elements 

-KV-store get API (1) - 1 points
-KV-store put API (1) - 1 point
-KV-store scan API (1) - 2 point
-In-memory memtable as balanced binary tree (1) - 4 points
-SSTs in storage with efficient binary search (1) - 3 points
-Database open and close API (1) - 2 points
-Extendible hash buffer pool (2) - 6 points
-Integration buffer with get (2) - 2 points
-shrink API (2) - 2 point
-Clock eviction policy (2) - 4 points
-LRU eviction policy (2) - 4 points
-Static B-tree for SSTs (2) - 4 points
-Bloom filter for SST and integration with get (3) - 5 points
-Compaction/Merge of two trees (3) - 5 points
-Support update (3) - 3 points
-Support delete (3) - 4 points

List of Experiments

-Data volume VS put performance (1) - 3 points
-Data volume VS get performance (1) - 3 points
-Data Volume VS scan performance (1) - 3 points
-LRU vs Clock eviction policy with query throughput with changing buffer pool size (2) - 5 points
-Binary search vs B-tree index with query throughput with changing data size (2) - 5 points
-Put throughput with increasing data size (3) - 3 points
-Get throughput with increasing data size (3) - 3 points
-Scan throughput with increasing data size (3) - 3 points
-Get performance for changing bloom filter bits with growing data size (3) - 5 points

A few things about the bonus marks:

-If you get it, the project is still capped at 100 points
-Extra things do not necessarily get you bonus unless they provide new insights (new experiments) or improve the design (new or changed implementation)
-You get a bonus if grading becomes easier, for example by mentioning clearly what doesn’t work, or having a lot of suitable tests.

