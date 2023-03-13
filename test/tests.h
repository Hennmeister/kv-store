#include "../include/SimpleKVStore.h"
#include "../src/BufferPool/BufferPool.h"


// ===================== Buffer Pool Tests =========================
void simple_buffer_pool(SimpleKVStore db);

// ===================== Inner-workings Tests =========================

void memtable_puts_and_gets(SimpleKVStore db);
void memtable_puts_and_scans(SimpleKVStore db);

// ===================== User-facing Tests =========================

void sequential_puts_and_gets(SimpleKVStore db);
void sequential_puts_and_scans(SimpleKVStore db);
void update_keys(SimpleKVStore db);
void edge_case_values(SimpleKVStore db);
void close_and_recover(SimpleKVStore db);
void multiple_dbs(SimpleKVStore db);
void simple_test(SimpleKVStore db);