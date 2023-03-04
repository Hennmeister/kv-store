
#include <string>

void print_file(string directory, string file_name);
void assert_val_equals(int val, int target, std::string test_name);
void assert_vec_equals(std::vector<std::pair<int, int>> val, std::vector<std::pair<int, int>> target, std::string test_name);