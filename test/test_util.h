#include <string>
#include <vector>
#include "../include/constants.h"

extern std::string test_dir;

void print_file(std::string directory, std::string file_name);
void assert_val_equals(int val, int target, std::string test_name);
void assert_vec_equals(std::vector<std::pair<int, int>> val, std::vector<std::pair<int, int>> target, std::string test_name);
void assert_buf_equals(uint8_t in_buf[PAGE_SIZE], uint8_t out_buf[PAGE_SIZE], std::string test_name);