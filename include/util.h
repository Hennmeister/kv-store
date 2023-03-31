#include <string>
#include <vector>

#ifndef KV_STORE_UTIL_H
#define KV_STORE_UTIL_H

int dir_exists(std::string dir);
char* string_to_char(std::string);
int safe_write(int fd, void* buf, long nbyte, long offset);
int safe_read(int fd, void* buf, long nbyte, long offset);
std::vector<std::pair<int, int>> priority_merge(std::vector<std::pair<int, int>> master,
                                                std::vector<std::pair<int, int>> older);

void print_data(const std::vector<std::pair<int, int>>& data);

void pad_data(std::vector<std::pair<int, int>>& src, int size);
#endif //KV_STORE_UTIL_H
