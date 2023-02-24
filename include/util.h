#include <string>

#ifndef KV_STORE_UTIL_H
#define KV_STORE_UTIL_H

char* string_to_char(std::string);
int safe_write(int fd, void* buf, long nbyte, long offset);
int safe_read(int fd, void* buf, long nbyte, long offset);

#endif //KV_STORE_UTIL_H
