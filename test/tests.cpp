
#include "./test_util.h"
#include "../include/SimpleKVStore.h"
#include "../include/constants.h"
#include "../include/Base/BufferPool.h"
#include "../include/BufferPool/BufferPoolEntry.h"
#include "../include/BufferPool/Directory.h"
#include "../include/BufferPool/LRUBuffer/LRUBuffer.h"
#include "../include/BufferPool/ClockBuffer/ClockBuffer.h"

#include <string>
#include <vector>
#include <cassert>

#include <iostream>

using namespace std;

// TODO: test updating pages
// ===================== LRU Pool Tests =========================
void simple_LRU_buffer(SimpleKVStore db)
{
    LRUBuffer *LRU_cache = new LRUBuffer(1, 10);
    // test inserting and retrieving 10 pages
    uint8_t in_buf[PAGE_SIZE];
    uint8_t out_buf[PAGE_SIZE];

    for (int i = 1; i <= 10; i++) {
        // fill in buf with page num
        for (int j = 0; j < PAGE_SIZE; j++) {
            in_buf[j] = (u_int8_t) i;
        }

        LRU_cache->put(i, in_buf);
        LRU_cache->get(i, out_buf);

        assert_buf_equals(in_buf, out_buf, "simple_buffer_pool");
    }
}

void LRU_simple_evict(SimpleKVStore db) {
    LRUBuffer *LRU_cache = new LRUBuffer(1, 1);
    uint8_t in_buf[PAGE_SIZE];
    uint8_t out_buf[PAGE_SIZE];

    // insert one more page than the buffer can hold
    for (int i = 1; i <= (MB / sizeof(LRUBufferEntry)) + 1; i++) {
        // fill in buf with page num

        in_buf[0] = i;

        LRU_cache->put(i, in_buf);
    }

    // check that the first value (1) was evicted
    assert_val_equals(LRU_cache->get(1, out_buf), -1, "LRU_simple_evict");
    // check the second inserted value was not evicted
    assert_val_equals(LRU_cache->get(2, out_buf), 0, "LRU_simple_evict");
    assert_val_equals(out_buf[0], 2, "LRU_simple_evict");
}

// test the referencing a value updates its recency and prevents it from being evicted
void LRU_ref_evict(SimpleKVStore db) {
    LRUBuffer *LRU_cache = new LRUBuffer(1, 1);
    uint8_t in_buf[PAGE_SIZE];
    uint8_t out_buf[PAGE_SIZE];

    // fill cache
    for (int i = 1; i <= (MB / sizeof(LRUBufferEntry)); i++) {
        in_buf[0] = i;
        LRU_cache->put(i, in_buf);
    }

    // reference and check that the first value is present
    assert_val_equals(LRU_cache->get(1, out_buf), 0, "LRU_ref_evict");

    // insert new page
    LRU_cache->put((MB / sizeof(LRUBufferEntry)) + 1, in_buf);

    // check that the first value is present
    assert_val_equals(LRU_cache->get(1, out_buf), 0, "LRU_ref_evict");

    // check the second inserted value was evicted
    assert_val_equals(LRU_cache->get(2, out_buf), -1, "LRU_ref_evict");
}

// test that we can grow the directory to the max size
void LRU_grow(SimpleKVStore db) {
    LRUBuffer *LRU_cache = new LRUBuffer(1, 10);
    uint8_t in_buf[PAGE_SIZE];
    uint8_t out_buf[PAGE_SIZE];

    for (int i = 1; i <= (MB / sizeof(LRUBufferEntry)) * 5; i++) {
        // fill in buf with page num
        in_buf[0] = i;
        LRU_cache->put(i, in_buf);
    }

    // check that every inserted page can still be found
    for (int i = 1; i <= (MB / sizeof(LRUBufferEntry)) * 5; i++) {
        assert_val_equals(LRU_cache->get(i, out_buf), 0, "LRU_max_grow");
        assert_val_equals(out_buf[0], (std::uint8_t ) i, "LRU_max_grow");
    }
}

void LRU_shrink(SimpleKVStore db) {
    LRUBuffer *LRU_cache = new LRUBuffer(1, 10);
    uint8_t in_buf[PAGE_SIZE];
    uint8_t out_buf[PAGE_SIZE];

    for (int i = 1; i <= (5 * MB / sizeof(LRUBufferEntry)); i++) {
        in_buf[0] = i;
        LRU_cache->put(i, in_buf);
    }


    LRU_cache->set_max_size(1);


    // check that every inserted page in the last MB can still be found
    for (int i = (4 * MB / sizeof(LRUBufferEntry)) + 1; i <= (5 * MB / sizeof(LRUBufferEntry)); i++) {
        assert_val_equals(LRU_cache->get(i, out_buf), 0, to_string(i));
        assert_val_equals(out_buf[0], (std::uint8_t ) i, "LRU_max_grow");
    }

    // check that all other pages were evicted
    for (int i = 1; i <= (4 * MB / sizeof(LRUBufferEntry)); i++) {
        assert_val_equals(LRU_cache->get(i, out_buf), -1, to_string(i));
    }

    LRU_cache->set_max_size(5);

    // check re-growing
    for (int i = 1; i <= (2 * MB / sizeof(LRUBufferEntry)); i++) {
        in_buf[0] = i;
        LRU_cache->put(i, in_buf);
    }
    for (int i = 1; i <= (2 * MB / sizeof(LRUBufferEntry)); i++) {
        assert_val_equals(LRU_cache->get(i, out_buf), 0, to_string(i));
    }
    for (int i = (4 * MB / sizeof(LRUBufferEntry)) + 1; i <= (5 * MB / sizeof(LRUBufferEntry)); i++) {
        assert_val_equals(LRU_cache->get(i, out_buf), 0, to_string(i));
        assert_val_equals(out_buf[0], (std::uint8_t ) i, "LRU_max_grow");
    }
}

// ===================== Clock Pool Tests =========================
void simple_clock_buffer(SimpleKVStore db)
{
    ClockBuffer *clock_cache = new ClockBuffer(1, 10);
    // test inserting and retrieving 10 pages
    uint8_t in_buf[PAGE_SIZE];
    uint8_t out_buf[PAGE_SIZE];

    for (int i = 1; i <= 10; i++) {
        // fill in buf with page num
        for (int j = 0; j < PAGE_SIZE; j++) {
            in_buf[j] = (u_int8_t) i;
        }

        clock_cache->put(i, in_buf);
        clock_cache->get(i, out_buf);

        assert_buf_equals(in_buf, out_buf, "simple_buffer_pool");
    }
}

void clock_simple_evict(SimpleKVStore db) {
    ClockBuffer *clock_cache = new ClockBuffer(1, 1);
    uint8_t in_buf[PAGE_SIZE];
    uint8_t out_buf[PAGE_SIZE];

    // insert one more page than the buffer can hold
    for (int i = 1; i <= (MB / sizeof(ClockBufferEntry)) + 1; i++) {
        // fill in buf with page num
        in_buf[0] = i;
        clock_cache->put(i, in_buf);
    }

    // check that one value was evicted
    int num_misses = 0;
    for (int i = 1; i <= (MB / sizeof(ClockBufferEntry)); i++) {
        if (clock_cache->get(i, out_buf) == -1) {
            num_misses++;
        }
    }
    assert_val_equals(num_misses, 1, "num misses");
}


// ===================== Inner-workings Tests =========================

void memtable_puts_and_gets(SimpleKVStore db)
{
    // TODO
}

void memtable_puts_and_scans(SimpleKVStore db)
{
    // TODO
}

// TODO: specific SST testing?

// ===================== User-facing Tests =========================

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
