
#include "./test_util.h"
#include "../include/SimpleKVStore.h"
#include "../include/constants.h"
#include "../include/Base/BufferPool.h"
#include "../include/BufferPool/BufferPoolEntry.h"
#include "../include/BufferPool/Directory.h"
#include "../include/BufferPool/LRUBuffer/LRUBuffer.h"
#include "../include/BufferPool/ClockBuffer/ClockBuffer.h"
#include "../src/BloomFilter/BloomFilter.h"

#include <algorithm>
#include <random>

#include <string>
#include <vector>
#include <cassert>

#include <iostream>

using namespace std;

// ===================== Bloom Filter ========================= //
void bloom_filter_simple(SimpleKVStore db) {
    BloomFilter *filter = new BloomFilter(10, 10);
    assert_val_equals(filter->testMembership(1), false, "bloom_filter_simple");
    filter->insert(1);
    assert_val_equals(filter->testMembership(1), true, "bloom_filter_simple");
}

// ===================== LRU Pool Tests ========================= //
void simple_LRU_buffer(SimpleKVStore db)
{
    LRUBuffer *LRU_cache = new LRUBuffer(1, 10);
    // test inserting and retrieving 10 pages
    uint8_t in_buf[PAGE_SIZE];
    uint8_t out_buf[PAGE_SIZE];

    for (int i = 1; i <= 10; i++) {
        // fill in buf with entry_data num
        for (int j = 0; j < PAGE_SIZE; j++) {
            in_buf[j] = (u_int8_t) i;
        }
        LRU_cache->put(to_string(i), in_buf, PAGE_SIZE);
        LRU_cache->get(to_string(i), out_buf);

        assert_buf_equals(in_buf, out_buf, "simple_LRU_buffer");
    }
}

void LRU_simple_evict(SimpleKVStore db) {
    LRUBuffer *LRU_cache = new LRUBuffer(1, 1, 0.25, 1);
    uint8_t in_buf[PAGE_SIZE];
    uint8_t out_buf[PAGE_SIZE];

    // insert one more entry_data than the buffer can hold
    for (int i = 1; i <= (MB / PAGE_SIZE) + 1; i++) {
        // fill in buf with entry_data num

        in_buf[0] = i;

        LRU_cache->put(to_string(i), in_buf, PAGE_SIZE);
    }

    // check that the first value (1) was evicted
    assert_val_equals(LRU_cache->get(to_string(1), out_buf), false, "LRU_simple_evicted");
    // check the second inserted value was not evicted
    assert_val_equals(LRU_cache->get(to_string(2), out_buf), true, "LRU_simple_not_evicted");
    assert_val_equals(out_buf[0], 2, "LRU_simple_evict_buf");
}

// test the referencing a value updates its recency and prevents it from being evicted
void LRU_ref_evict(SimpleKVStore db) {
    LRUBuffer *LRU_cache = new LRUBuffer(1, 1, 0.25, 1);
    uint8_t in_buf[PAGE_SIZE];
    uint8_t out_buf[PAGE_SIZE];

    // fill cache
    for (int i = 1; i <= (MB / PAGE_SIZE); i++) {
        in_buf[0] = i;
        LRU_cache->put(to_string(i), in_buf, PAGE_SIZE);
    }

    // reference and check that the first value is present
    assert_val_equals(LRU_cache->get(to_string(1), out_buf), true, "LRU_ref_evict");

    // insert new entry_data
    LRU_cache->put(to_string((MB / PAGE_SIZE) + 1), in_buf, PAGE_SIZE);

    // check that the first value is present
    assert_val_equals(LRU_cache->get(to_string(1), out_buf), true, "LRU_ref_evict");

    // check the second inserted value was evicted
    assert_val_equals(LRU_cache->get(to_string(2), out_buf), false, "LRU_ref_evict");
}

// test that we can grow the directory to the max size
void LRU_grow(SimpleKVStore db) {
    LRUBuffer *LRU_cache = new LRUBuffer(1, 10, 0.25, 1);
    uint8_t in_buf[PAGE_SIZE];
    uint8_t out_buf[PAGE_SIZE];

    for (int i = 1; i <= (MB / PAGE_SIZE) * 5; i++) {
        // fill in buf with entry_data num
        in_buf[0] = i;
        LRU_cache->put(to_string(i), in_buf, PAGE_SIZE);
    }

    // check that every inserted entry_data can still be found
    for (int i = 1; i <= (MB / PAGE_SIZE) * 5; i++) {
        assert_val_equals(LRU_cache->get(to_string(i), out_buf), true, "LRU_max_grow");
        assert_val_equals(out_buf[0], (std::uint8_t ) i, "LRU_max_grow");
    }
}

void LRU_shrink(SimpleKVStore db) {
    LRUBuffer *LRU_cache = new LRUBuffer(1, 10, 0.25, 1);
    uint8_t in_buf[PAGE_SIZE];
    uint8_t out_buf[PAGE_SIZE];

    for (int i = 1; i <= (5 * MB / PAGE_SIZE); i++) {
//        cout << i << endl;
        in_buf[0] = i;
        LRU_cache->put(to_string(i), in_buf, PAGE_SIZE);
    }

    LRU_cache->set_max_size(1);

    // check that every inserted entry_data in the last MB can still be found
    for (int i = (4 * MB / PAGE_SIZE) + 1; i <= (5 * MB / PAGE_SIZE); i++) {
        assert_val_equals(LRU_cache->get(to_string(i), out_buf), true, to_string(i));
        assert_val_equals(out_buf[0], (std::uint8_t ) i, "LRU_max_grow");
    }

    // check that all other pages were evicted
    for (int i = 1; i <= (4 * MB / PAGE_SIZE); i++) {
        assert_val_equals(LRU_cache->get(to_string(i), out_buf), false, to_string(i));
    }

    LRU_cache->set_max_size(5);

    // check re-growing
    for (int i = 1; i <= (2 * MB / PAGE_SIZE); i++) {
        in_buf[0] = i;
        LRU_cache->put(to_string(i), in_buf, PAGE_SIZE);
    }
    for (int i = 1; i <= (2 * MB / PAGE_SIZE); i++) {
        assert_val_equals(LRU_cache->get(to_string(i), out_buf), true, to_string(i));
    }
    for (int i = (4 * MB / PAGE_SIZE) + 1; i <= (5 * MB / PAGE_SIZE); i++) {
        assert_val_equals(LRU_cache->get(to_string(i), out_buf), true, to_string(i));
        assert_val_equals(out_buf[0], (std::uint8_t ) i, "LRU_max_grow");
    }
}

// ===================== Clock Pool Tests =========================
void simple_clock_buffer(SimpleKVStore db)
{
    ClockBuffer *clock_cache = new ClockBuffer(1, 10, 0.25, 1);
    // test inserting and retrieving 10 pages
    uint8_t in_buf[PAGE_SIZE];
    uint8_t out_buf[PAGE_SIZE];

    for (int i = 1; i <= 10; i++) {
        // fill in buf with entry_data num
        for (int j = 0; j < PAGE_SIZE; j++) {
            in_buf[j] = (u_int8_t) i;
        }

        clock_cache->put(to_string(i), in_buf, PAGE_SIZE);
        clock_cache->get(to_string(i), out_buf);

        assert_buf_equals(in_buf, out_buf, "simple_buffer_pool");
    }
}

void clock_simple_evict(SimpleKVStore db) {
    ClockBuffer *clock_cache = new ClockBuffer(1, 1, 0.25, 1);
    uint8_t in_buf[PAGE_SIZE];
    uint8_t out_buf[PAGE_SIZE];

    // insert one more entry_data than the buffer can hold
    for (int i = 1; i <= (MB / PAGE_SIZE) + 1; i++) {
        // fill in buf with entry_data num
        in_buf[0] = i;
        clock_cache->put(to_string(i), in_buf, PAGE_SIZE);
    }

    // check that one value was evicted
    int num_misses = 0;
    for (int i = 1; i <= (MB / PAGE_SIZE); i++) {
        if (clock_cache->get(to_string(i), out_buf) == false) {
            num_misses++;
        }
    }
    assert_val_equals(num_misses, 1, "num misses");
}


// ===================== Inner-workings Tests =========================

void memtable_puts_and_gets(SimpleKVStore db)
{
    for (int i = 0; i < PAGE_NUM_ENTRIES; i++)
    {
        db.put(i, -i);
    }

    int val;
    for (int i = 0; i < PAGE_NUM_ENTRIES; i++)
    {
        db.get(i, val);
        assert_val_equals(val, -i, "memtable_puts_and_gets");
    }
}

// ===================== User-facing Tests =========================
void random_puts_and_gets(SimpleKVStore db)
{
    vector<int> data = vector<int>();
    vector<int> insertion_order = vector<int>();

    auto rng = std::default_random_engine {};

    int test_size = 100 * PAGE_NUM_ENTRIES + 300;
    for (int i = 0; i < test_size; i++)
    {
        insertion_order.push_back(i);
        data.push_back(rand() % 10000000);
    }
    std::shuffle(std::begin(insertion_order), std::end(insertion_order), rng);

    int key;
    for (int i = 0; i < test_size; i++)
    {
        key = insertion_order[i];
        db.put(key, data[key]);
    }

    int val;
    for (int i = 0; i < test_size; i++)
    {
        key = rand() % test_size;
        db.get(key, val);
        assert_val_equals(val, data[key], "random_puts_and_gets");
    }
}



void sequential_puts_and_gets(SimpleKVStore db)
{

    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++)
    {
        db.put(i, -i);
    }

    int val;
    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++)
    {
        db.get(i, val);
        assert_val_equals(val, -i, "sequential_puts_and_gets");
    }
}

void hash_test(SimpleKVStore db){
    int hasha, hashb;
    int seed = 1230981234;
    int val = 1230;

    MurmurHash3_x86_32((void *) &val, sizeof(int), seed, (void *) &hasha);
    MurmurHash3_x86_32((void *) &val, sizeof(int), seed, (void *) &hashb);

    assert(hasha == hashb);
}

void sequential_puts_and_scans(SimpleKVStore db)
{

    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++)
    {
        db.put(i, -i);
    }

    vector<pair<int, int>> key_vals{};
    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++)
    {
        key_vals.push_back(pair<int, int>({i, -i}));
        vector<pair<int, int>> scan = db.scan(0, i);
        assert_vec_equals(scan, key_vals, "sequential_puts_and_scans_1");
    }

    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++)
    {
        vector<pair<int, int>> scan = db.scan(i, 3 * PAGE_NUM_ENTRIES + 300 - 1);
        assert_vec_equals(scan, key_vals, "sequential_puts_and_scans_2");
        key_vals.erase(key_vals.begin());
    }
}

void update_keys(SimpleKVStore db)
{
    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++)
    {
        db.put(i, -i);
    }

    int val;
    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++)
    {
        db.get(i, val);
        assert_val_equals(val, -i, "update_keys");
    }

    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++)
    {
        db.put(i, -i * 2);
    }

    vector<pair<int, int>> key_vals{};
    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++)
    {
        db.get(i, val);
        key_vals.emplace_back(i, -i*2);
        assert_val_equals(val, -i * 2, "update_keys");
    }
    vector<pair<int, int>> scan = db.scan(0, 3 * PAGE_NUM_ENTRIES + 300 - 1);
    assert_vec_equals(scan, key_vals, "update_keys");

}

void delete_keys(SimpleKVStore db)
{
    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++)
    {
        db.put(i, -i);
    }

    int val;
    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++)
    {
        db.get(i, val);
        assert_val_equals(val, -i, "delete_keys");
    }

    // Delete every other key
    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i += 2)
    {
        db.delete_key(i);
    }

    vector<pair<int, int>> key_vals{};
    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++)
    {
        int status = db.get(i, val);
        if (i % 2 == 0)
            assert_val_equals(status, false, "delete_keys_false");
        else
            assert_val_equals(status, true, "delete_keys_true");
    }
}

void edge_case_values(SimpleKVStore db)
{
    assert_vec_equals(db.scan(5, 4), vector<pair<int, int>>{}, "edge_case_values1");
}

void close_and_recover(SimpleKVStore db)
{
    db.close();

    db.open(test_dir + "shared_db");

    // Test gets
    int val;
    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++)
    {
        db.get(i, val);
        assert_val_equals(val, -i, "close_and_recover_gets");
    }

    // Test scans
    vector<pair<int, int>> key_vals{};
    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++)
    {
        key_vals.push_back(pair<int, int>({i, -i}));
        vector<pair<int, int>> scan = db.scan(0, i);
        assert_vec_equals(scan, key_vals, "close_and_recover_scans_1");
    }

    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++)
    {
        vector<pair<int, int>> scan = db.scan(i, 3 * PAGE_NUM_ENTRIES + 300 - 1);
        assert_vec_equals(scan, key_vals, "close_and_recover_scans_2");
        key_vals.erase(key_vals.begin());
    }
}

void multiple_dbs(SimpleKVStore db)
{
    SimpleKVStore db1;
    SimpleKVStore db2;
    SimpleKVStore db3;
    db1.open(test_dir + "db1");
    db2.open(test_dir + "db2");
    db3.open(test_dir + "db3");

    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++)
    {
        db1.put(i, i);
        db2.put(i, i + 1);
        db3.put(i, i + 2);
    }

    int val;
    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++)
    {
        db1.get(i, val);
        assert_val_equals(val, i, "multiple_dbs_db1");
        db2.get(i, val);
        assert_val_equals(val, i + 1, "multiple_dbs_db2");
        db3.get(i, val);
        assert_val_equals(val, i + 2, "multiple_dbs_db3");
    }
    db1.close();
    db2.close();
    db3.close();
}

void simple_test(SimpleKVStore db)
{
    db.put(1, 1);
    db.put(-2, -2);
    db.put(5, 5);
    int val = 0;
    db.get(1, val);
    assert(val == 1);
    db.get(-2, val);
    assert(val == -2);
    db.get(5, val);
    assert(val == 5);
    assert(db.get(-1, val) == false);
    db.put(1, 10);
    db.get(1, val);
    assert(val == 10);
}
