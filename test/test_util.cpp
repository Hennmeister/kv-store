#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include "test_util.h"

using namespace std;

void print_file(string directory, string file_name)
{
    ifstream *read_file = new ifstream();
    read_file->open(directory + "/" + file_name, ios::in | ios::binary);

    if (!read_file->is_open())
        throw runtime_error(directory + "/" + file_name + " data file could not be opened");

    ofstream *write_file = new ofstream();
    write_file->open(directory + "/" + file_name + "_debug.txt", ios::binary | ios::app);

    if (!write_file->is_open())
        throw runtime_error(directory + "/" + file_name + "_debug.txt" + " data file could not be opened");

    while (!read_file->eof())
    {
        int32_t first, second;
        read_file->read(reinterpret_cast<char *>(&first), sizeof(int32_t));
        read_file->read(reinterpret_cast<char *>(&second), sizeof(int32_t));
        (*write_file) << to_string(first) + ", " << to_string(second) << endl;
    }
}

void assert_val_equals(int val, int target, string test_name)
{
    if (val != target)
    {
        throw runtime_error(test_name + ": val != target --> " + to_string(val) + " != " + to_string(target));
    }
}

void assert_vec_equals(vector<pair<int, int>> val, vector<pair<int, int>> target, string test_name)
{
    if (val != target)
    {
        cout << endl;

        cout << "val: ";
        for (pair<int, int> i : val)
            cout << to_string(i.first) << "," << to_string(i.second) << ' ';

        cout << endl;

        cout << "target: ";
        for (pair<int, int> i : target)
            cout << to_string(i.first) << "," << to_string(i.second) << ' ';

        cout << endl;

        throw runtime_error(test_name + ": val != target --> Sizes: " + to_string(val.size()) + " != " + to_string(target.size()));
    }
}

void assert_buf_equals(uint8_t in_buf[PAGE_SIZE], uint8_t out_buf[PAGE_SIZE], std::string test_name)
{
    for (int i = 0; i < PAGE_SIZE; i++) {
        if (in_buf[i] != out_buf[i]) {
            throw runtime_error(test_name + ": in_buf != out_buf --> mismatch at index " + to_string(i) +
            " : " +  to_string(in_buf[i]) + " != " + to_string(out_buf[i]));
        }
    }
}