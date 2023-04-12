# KV Store Report

Refer to [this page](https://docs.google.com/document/d/1dsIuIzXiIBbiZcNYi1cC62PVE4mF-MdksmIQ1hikFGM) for the official project handout.

# Table of Contents
1. [Introduction](#introduction)
2. [Compilation](#compilation)
    1. [Tests](#tests)
    2. [Playground](#playground)
    3. [Experiments](#experiments)
3. [Project Status](#status)
4. [Database Initialization and Parameters](#init_param)
    1. [DbOptions](#dboptions)
5. [Implementation Steps](#steps)
    1. [Abstractions](#abstractions)
    2. [Step 1](#step1)
    3. [Step 2](#step2)
    4. [Step 3](#step3)
6. [Testing](#testing)


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

## "I don't care, just tell me how to run it." - Fine... <a name="compilation"></a>

Run `./make.sh` to compile all executables and run the tests in one command. 

### Tests

The test executable file will be located under `/build/kv-store-test` and this file runs all unit tests.

### Playground

We provide an "empty" C++ file with a default version of our database (that you are free modify if you wish - see [Database Initialization and Parameters](#database-initialization-and-parameters)) for you to play with and write any calls to `open`, `put`, `get`, `scan`, print results and verify functionality. The aim is to provide you with an easy way to interact with our database without having to compile a new project. The executable can be found at `/build/kv-store-test` and the file can be changed at `/playground.cpp`.

### Experiments

Run `./experiments.sh` to run the executable that generates all experiments data. You can also generate data individually for each experiment by calling the executable file `/build/kv-store-performance-test` with the parameters indicated on the calls of `experiments.sh`. We also provide a `plot_experiments.sh` script that plots the data generated for each experiment. You can also plot the data of individual experiments by using the same approach.

## Project Status <a name="status"></a>

TODO: at the end

## Database Initialization and Parameters <a name="init_param"></a>

We provide the user with a DbOptions object that is used to set default configurations for any database instantiated, and also giving the freedom to specify some options based on the user's preference.

As such, open("database name") is a valid call to create a database if the user simply wants a functioning KV-store without thinking about possible parameters/options, but they may also generate a DbOptions object and pass that in as a second parameter to the call for further customization.

### DbOptions

**Default values**

    - Memtable
        - memTableType: "RedBlackTree"
        - maxMemtableSize: 10 * MEGABYTE

    - SST
        - sstManager: "LSMTreeManager"
        - sstSearch: "BTree"
        - btreeFanout: 100

    - Buffer Pool
        - bufferPoolType: "LRU"
        - bufferPoolMinSize: 1
        - bufferPoolMaxSize: 10

    - Bloom filter
        - filterBitsPerEntry: 10

**Avalable Options**

    - Memtable
        - memTableType: "RedBlackTree"
        - maxMemtableSize: any positive integer multiple of ENTRY_SIZE (representing the maximum number of bytes stored in the memtable)

    - SST
        - sstManager: "BTreeManager", "LSMTreeManager"
        - sstSearch: "BTree", "BinarySearh"
        - btreeFanout: any positive integer value (representing the fanout of the btree)

    - Buffer Pool
        - bufferPoolType: "Clock", "LRU", "None"
        - bufferPoolMinSize: any non-negative integer value (representing the minimum size of buffer pool in MB)
        - bufferPoolMaxSize: any positive integer value (representing the maximum size of buffer pool in MB)

    - Bloom filter
        - filterBitsPerEntry: any non-negative integer value (representing the number of bits per entry in the Bloom filterff)

**Example**

````md
    DbOptions *options = new DbOptions();
    options->setSSTSearch("LSMTree");
    options->setBufferPoolSize(0, 10); // min size, max size
    options->setFilterBitsPerEntry(10);
    options->setMaxMemtableSize(1 * MEGABYTE);
    
    SimpleKVStore db;
    db.open("<db_path>/<db_name>", options);
````
---
## Implementation Steps <a name="steps"></a>

Here we outline the process of implementation the various parts of our system. For simplicity, our simple KV-Store only handles integer keys and integer values.

The general flow is the following: entries get populated in a memtable (fitting entirely in memory) that holds the most recent key-value insertions in the database. Once the memtable grows beyond its capacity, the contents of the memtable are dumped to an SST sorted by the keys.

### Abstractions

Aware of the possible changes to the algorithms as well as our methodology, either as a consequence of efficiency tradeoffs or improvements to the system, we tried to structure our OOP code in a way that maximizes the use of abstractions/interfaces while minimizing the amount of coupling our classes have between each other.

We have a few base interfaces that can be found under the `/include/Base` directory of the project.


### Utility <a name="utility"></a>

- `priority_merge` - This function is a core utility function used throughout our implementation. It allows the neat compaction of various sources of data in order to produce output data which prioritizes the newest data. The function takes in 2 inputs, 1 set of newer data (key-value pairs) and one set of older data. If there is ever an entry which is present in both sources, the function will use the newer version of the key. An example of usage would be when priority merging data from the memtable with data found in SSTs. 

### High-Level OOP Diagram

For better understanding of the codebase and our design decisions, we include here a diagram of our main classes and how they interact with each other:

TODO: image


### Step 1 <a name="step1"></a>

- **Memtable**

We implement a memtable as a balanced binary search tree ([red-black tree](https://en.wikipedia.org/wiki/Red%E2%80%93black_tree) to be precise) supporting standard `put(key,val)`, `get(key)` and `scan(key1, key2)` methods. We also implement an inorder tree traversal that returns all the key-value pairs sorted by their keys, which will be helpful to dump the content into the corresponding SST (refer to next bullet point).

- **SSTs (Sorted String Tables)**

We set a maximum capacity (e.g. a page size of 4KB) to the Memtable, at which point we dump the key-value pairs in sorted order to an SST file `x.sst`. The SSTs are thus stored in decreasing order of longevity where `1.sst` is the oldest Memtable dumped. On a get query that is not found on the current Memtable, our database traverses over the SSTs from newest to oldest to find a key. Note that we implement the SST dump so that it writes in binary to an append-only file to maximize efficiency in sequential writes.

Our initial implementation was quite raw and assumed that a new SST File was made every time a memtable was dumped. This implies that each operation performed on any SST loaded from disk could be performed in memory seeing as they do not grow in size. This assumption is later relaxed as we introduce more complications to our implementation. 

In order to ensure efficiency, we use the `O_DIRECT` flag when opening a file and ensure that all reads are multiples of 512. 

#### Step 1 Experiments

This experiment aims to measure the throughput of the put, get, and scan operations. The methodology is as follows:

For each iteration, we:

1. Randomly sample STEP_SIZE keys and `put` those keys in the db. Time and average throughput for that iteration.
2. Randomly sample NUM_QUERIES keys and `get` those keys from the db. Time and average throughput for that iteration.
3. Randomly sample NUM_QUERIES keys and `get` those keys from the db. Time and average throughput for that iteration.

Note that "randomly" in this case is not uniform. Instead our sample has an intentional skew towards lower valued keys to more closely simulate a real database workload.

At each iteration, since we increase the total database size at every step, NUM_QUERIES is calcaulated from a percentage of all the keys inserted into the database at that point.
 
The graphs are saved under the name experiment1.png and shown below:

TODO: image

### Step 2 <a name="step2"></a>

- **Buffer Pool**

TODO

- **Eviction Policies**

TODO

- **B-Tree for SST**

In order to execute a BTree correctly, we increased the complexity of our program by now introducing the SSTFileManager class that adds a layer of abstraction and allows the BufferPool to not have to interface directly with any SSTManager. The BTree SST was implemented in stages, the first of which being the exact same as the Append Only SST. In this stage, the data on disk was the exact same but every time an SST was loaded/created, an in-memory BTree was built, allowing Get/Scan calls to query the in memory structure before needing to access disk. 

This in-memory structure was simply a vector of vector of ints. Where each level of the BTree is represented by a vector of ints. Through some simple maths and the knowledge of the fannout (which was stored in the metadata page of each SST), the in-memory structure could be easily used to find the position or lower bound of an element for get or scan calls. 

Slowly, we transitioned to having these in memory structures being written out as "internal node pages" where this data could be parsed from. The data written to disk was exactly the vector of vector of ints, where each vector's end was delimited by an `INT_MAX - 1`. Now, the BTree was only constructed on first creation of the SST (i.e. from memtable to SST dump) and everytime the database was opened thereafter, the BTree was loaded from disk. 

By padding our internal node pages with blank data, we were able to ensure that the start of the leaves was always the start of a new page, this is important for the binary search as it ensures that no complicated processing has to be done to differentiate between internal node data and leaf data which would have different structures. 

After fully implementing the BTree with scans and gets, we then revisited the binary search function and upgraded our old append only file implementation of binary search to now work without needing to load the entire SST into memory. 

At this point, our BTree could function as both a large Append Only File or a BTree through the use of the `useBinary` option. 

_Note: The internal nodes are only read from once, on SST load. While we understand that these nodes should be handled as regular pages and read from disk each time, with sufficiently large fanouts, the number of integers in all internal nodes scales very very very well (log base fannout). This allows us to keep all internal ndoes in memory at all times._ 

#### Experiments

TODO: step2 experiments

### Step 3 <a name="step3"></a>

- **LSM Tree**

Since our BTree implementation had already been completed in the last step, this step was simply a case of creating a new style of SSTManager, the LSMSSTManager. This class would have to manage the various levels in memory, reconstruct the levels on load of an existing database, and perform compaction when required. 

Thankfully, our BTree implementation already managed to handle varying multiples of the memtable size quite well so no significant changes were required to be made to the BTree implementation. In order to determine the levels, we could simply look at the number of entries in each SST and use logarithms to figure out which level each SST belongs to. 

Finally, the most significant part of the LSM Tree implementation, compaction. In order to facilitate compaction, we first implemented the LSM Tree with an entirely in-memory compaction process. This allowed us to ensure that all other parts of the LSMSSTManager was working correctly. Once this was completed, the algorithm was planned out and implemented as follows:

1. From each of the 2 SSTs being passed in for compaction, we compute the maximum number of possible internal nodes that could be made as a result of compaction (simply by summing the total internal nodes of both SSTs and adding some padding), this allows us to know how many maximum internal node pages we could have.
2. An initial file is created to be appended to as the compaction process completes.
3. A page-by-page priority merge is performed, using 2 buffers 1 for each SST and 2 pointers to track the current position within a page. Once at least one page of data is produced, it is flushed to disk. When a page is about to be written out, we iterate over the page and append to the lowest level of the BTree.
4. With the lowest level of the BTree in memory, we now compute the upper levels and flush the BTree's internal nodes to disk. 
5. The metadata of the file is updated.
6. A new BTreeSST object is loaded and returned by providing the filename of the newly created SST

_Note: When scanning across pages, we do not store data in the buffer pool - this is to prevent sequential flooding_

- **Incorporating Updates and Deletes**

TODO

- **Bloom Filter**

Bloom filter file names are bloom_filter_{level}, where level is the level of the LSM tree for which this bloom filter tracks set membership.
TODO

#### Experiments

TODO: step3 experiments

## Testing

In our efforts to assure the quality of our code, we relied on unit tests to check individual isolated components, integrated tests to verify that all our components are correctly combined in the flow of the application, as well as a lot of manual testing on playground and on the larger experiments. Some of the unit/integration tests are included in the `src/kv-store-test.cpp` file and we include them here for reference:

- **simple_test:** a basic interaction with a database of putting, getting and updating a few values 
- **hash_test:** a simple test that a the hashing function used is consistent with the same value
- **memtable_puts_and_gets:** checks that memtables correctly stores and retrieves key-value pairs
- **sequential_puts_and_gets:** checks that the db correctly stores sequential keys and retrieves them on get calls
- **sequential_puts_and_scans:** checks that the db correctly stores sequential keys and retrieves them on scan calls
- **random_puts_and_gets:** checks that the db correctly stores random keys and retrieves them on get calls
- **update_keys:** checks that the db correctly updates keys corretly
- **edge_case_values:** checks for edge cases
- **multiple_dbs:** manages multiple dbs opened at once and ensure they are each correctly managed
- **simple_LRU_buffer:**
- **LRU_simple_evict:**
- **LRU_ref_evict:**
- **LRU_grow:**
- **LRU_shrink:**
- **simple_clock_buffer:**
- **clock_simple_evict:**
- **bloom_filter_simple:**

We also run the same tests with multiple database configurations (different search techniques, different buffer options, different sizes of various components) to ensure that the options behave consistenly across the board.





TODO: DELETE THE FOLLOWING

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

