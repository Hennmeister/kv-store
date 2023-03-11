#include "../include/SimpleKVStore.h"

// ===================== Inner-workings Tests =========================

void memtable_puts_and_gets(SimpleKVStore db);
void memtable_puts_and_scans(SimpleKVStore db);

// ===================== User-facing Tests =========================

void sequeantial_puts_and_gets(SimpleKVStore db);
void sequeantial_puts_and_scans(SimpleKVStore db);
void update_keys(SimpleKVStore db);
void edge_case_values(SimpleKVStore db);
void close_and_recover(SimpleKVStore db);
void multiple_dbs(SimpleKVStore db);
void simple_test(SimpleKVStore db);