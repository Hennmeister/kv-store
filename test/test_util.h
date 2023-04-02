
#include <string>
#include <vector>
#include "../include/constants.h"
#define MEMTABLE_TEST_SIZE (PAGE_NUM_ENTRIES * 2)


extern std::string test_dir;

void print_file(std::string directory, std::string file_name);
void assert_val_equals(int val, int target, std::string test_name);
void assert_vec_equals(std::vector<std::pair<int, int>> val, std::vector<std::pair<int, int>> target, std::string test_name);