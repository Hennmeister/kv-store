#include "../include/SimpleKVStore.h"
#include "../include/Base/BufferPool.h"

// ===================== Bloom filter Tests =========================
void bloom_filter_simple(SimpleKVStore db);

// ===================== Buffer Pool Tests =========================
void simple_LRU_buffer(SimpleKVStore db);
void LRU_simple_evict(SimpleKVStore db);
void LRU_ref_evict(SimpleKVStore db);
void LRU_shrink(SimpleKVStore db);
void LRU_grow(SimpleKVStore db);

void simple_clock_buffer(SimpleKVStore db);
void clock_simple_evict(SimpleKVStore db);
void hash_test(SimpleKVStore db);
void random_puts_and_gets(SimpleKVStore db);

// ===================== Inner-workings Tests =========================

void memtable_puts_and_gets(SimpleKVStore db);

// ===================== User-facing Tests =========================

void sequential_puts_and_gets(SimpleKVStore db);
void sequential_puts_and_scans(SimpleKVStore db);
void update_keys(SimpleKVStore db);
void delete_keys(SimpleKVStore db);
void edge_case_values(SimpleKVStore db);
void close_and_recover(SimpleKVStore db);
void multiple_dbs(SimpleKVStore db);
void simple_test(SimpleKVStore db);