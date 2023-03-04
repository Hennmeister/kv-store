
#include "./test_util.h"
#include "../include/SimpleKVStore.h"
#include "../include/constants.h"
#include <string>
#include <vector>
#include <cassert>

using namespace std;

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

void sequeantial_puts_and_gets(SimpleKVStore db)
{

    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++)
    {
        db.put(i, -i);
    }

    int val;
    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++)
    {
        db.get(i, val);
        assert_val_equals(val, -i, "sequeantial_puts_and_gets");
    }
}

void sequeantial_puts_and_scans(SimpleKVStore db)
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
        assert_vec_equals(scan, key_vals, "sequeantial_puts_and_scans_1");
    }

    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++)
    {
        vector<pair<int, int>> scan = db.scan(i, 3 * PAGE_NUM_ENTRIES + 300 - 1);
        assert_vec_equals(scan, key_vals, "sequeantial_puts_and_scans_2");
        key_vals.erase(key_vals.begin());
    }
}

void update_keys(SimpleKVStore db)
{
    // TODO: add a few keys, update a few keys and make sure they are updated/maintained properly
}

void edge_case_values(SimpleKVStore db)
{
    // assert_vec_equals(db.scan(5, 4), vector<pair<int, int>>{}, "edge_case_values1");
}

void close_and_recover(SimpleKVStore db)
{
    db.close();

    db.open(test_dir + "shared_db", PAGE_NUM_ENTRIES);

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
    db1.open(test_dir + "db1", PAGE_NUM_ENTRIES);
    db2.open(test_dir + "db2", PAGE_NUM_ENTRIES);
    db3.open(test_dir + "db3", PAGE_NUM_ENTRIES);

    for (int i = 0; i < 3 * PAGE_NUM_ENTRIES + 300; i++)
    {
        db1.put(i, i);
        db2.put(i, i + 1);
        db3.put(i, i + 2);
    }

    // TODO: gets and scans to ensure it remains the same
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
