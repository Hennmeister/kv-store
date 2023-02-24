#include <unistd.h>
#include "../include/util.h"

// Note that this REQUIRES the output to be free'd after.
char* string_to_char(std::string s){
    //https://www.geeksforgeeks.org/convert-string-char-array-cpp/

    const int length = s.length();

    // declaring character array (+1 for null terminator)
    char* char_array = new char[length + 1];

    // copying the contents of the
    // string to char array
    strcpy(char_array, s.c_str());
    return char_array;
}

int safe_write(int fd, void* buf, long nbyte, long offset){
    int res = pwrite(fd, buf, nbyte, offset);
    int write_completion = res;
    while(write_completion != nbyte){
        if(res < 0){
            perror("writing failed.");
            return write_completion;
        }
        write_completion += res;
        res = pwrite(fd, (char*)buf + write_completion, nbyte - write_completion, offset + write_completion);
    }
    return write_completion;
}


int safe_read(int fd, void* buf, long nbyte, long offset){
    int res = pread(fd, buf, nbyte, offset);
    int read_completion = res;
    while(read_completion != nbyte){
        if(res < 0){
            perror("reading failed.");
            return read_completion;
        }
        read_completion += res;
        res = pread(fd, (char*)buf + read_completion, nbyte - read_completion, offset + read_completion);
    }
    return read_completion;
}
