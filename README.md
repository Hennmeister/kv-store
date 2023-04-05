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

### Step2

- **Buffer Pool**

TODO

- **Eviction Policies**

TODO

- **B-Tree for SST**
Note: When do we use `get_page` and when do we use `get_pages` in order to buffer/not buffer calls?

#### Experiments

TODO: step2 experiments
