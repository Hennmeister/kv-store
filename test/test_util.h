
#include <string>
#include <vector>

extern std::string test_dir;

void print_file(std::string directory, std::string file_name);
void assert_val_equals(int val, int target, std::string test_name);
void assert_vec_equals(std::vector<std::pair<int, int>> val, std::vector<std::pair<int, int>> target, std::string test_name);