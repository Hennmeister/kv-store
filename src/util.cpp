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