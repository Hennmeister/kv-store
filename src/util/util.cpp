#include <unistd.h>
#include "../../include/util.h"
#include <iostream>
#include <climits>
#include <cstring>
#include <string>
#include <sys/stat.h>
#include <random>

using namespace std;

bool sortByFname(const std::pair<std::string,int> &a,
                 const std::pair<std::string,int> &b)
{
    int pos_a = stoi(a.first.substr(0, a.first.size()-4));
    int pos_b = stoi(b.first.substr(0, b.first.size()-4));
    return (pos_a < pos_b);
}

int dir_exists(std::string dir)
{
    struct stat info
    {
    };
    if (stat(dir.c_str(), &info) != 0)
        return 0;
    else if (info.st_mode & S_IFDIR)
        return 1;
    return -1;
}

int rand_int(int range_from, int range_to) {
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());
    std::uniform_int_distribution<int> unif_sample(range_from, range_to);
    return unif_sample(generator);
}

void pad_data(std::vector<std::pair<int, int>> &src, int size)
{
    if (src.size() >= size)
    {
        return;
    }
    int original_size = src.size();
    for (int i = 0; i < size - original_size; i++)
    {
        src.emplace_back(pair<int, int>({INT_MAX, 0}));
    }
}

void print_data(const vector<pair<int, int>> &data)
{
    for (auto &i : data)
    {
        cout << "key: " << i.first << " value: " << i.second << endl;
    }
}

// priority merge -> Include all elements from both lists, if an element exists in both lists, use only the element
// from the master list (this alludes to keeping the latest entry)
std::vector<std::pair<int, int>> priority_merge(std::vector<std::pair<int, int>> master,
                                                std::vector<std::pair<int, int>> older)
{
    int ind0 = 0;
    int ind1 = 0;
    auto res = vector<pair<int, int>>();

    while (ind0 < master.size() && ind1 < older.size())
    {
        if (master[ind0].first > older[ind1].first)
        {
            res.emplace_back(older[ind1]);
            ind1++;
        }
        else if (master[ind0].first < older[ind1].first)
        {
            res.emplace_back(master[ind0]);
            ind0++;
        }
        else
        {
            if(master[ind0].second != INT_MIN) {
                res.emplace_back(master[ind0]);
            }
            ind0++;
            ind1++;
        }
    }
    if (ind0 != master.size())
    {
        for (int i = ind0; i < master.size(); i++)
        {
            res.emplace_back(master[i]);
        }
    }
    if (ind1 != older.size())
    {
        for (int i = ind1; i < older.size(); i++)
        {
            res.emplace_back(older[i]);
        }
    }

    return res;
}

// Note that this REQUIRES the output to be free'd after.
char *string_to_char(std::string s)
{
    // https://www.geeksforgeeks.org/convert-string-char-array-cpp/

    const int length = s.length();

    // declaring character array (+1 for null terminator)
    char *char_array = new char[length + 1];

    // copying the contents of the
    // string to char array
    strcpy(char_array, s.c_str());
    return char_array;
}

int safe_write(int fd, void *buf, long nbyte, long offset)
{
    int res = pwrite(fd, buf, nbyte, offset);
    int write_completion = res;
    while (write_completion != nbyte)
    {
        if (res < 0)
        {
            perror("writing failed.");
            return write_completion;
        }
        write_completion += res;
        res = pwrite(fd, (char *)buf + write_completion, nbyte - write_completion, offset + write_completion);
    }
    return write_completion;
}

int safe_read(int fd, void *buf, long nbyte, long offset)
{
    int res = pread(fd, buf, nbyte, offset);
    int read_completion = res;
    while (read_completion < nbyte)
    {
        if (res < 0)
        {
            perror("reading failed.");
            return read_completion;
        }
        read_completion += res;
        res = pread(fd, (char *)buf + read_completion, nbyte - read_completion, offset + read_completion);
    }
    return read_completion;
}


int binary_search(std::vector<std::pair<int, int>> data, int target, int &value){
    int left = 0;
    int right = data.size();
    while (left <= right)
    {
        int mid = left + (right - left) / 2;

        int key = data[mid].first;

        if (key == target)
        {
            value = data[mid].second;
            return mid;
        }
        else if (key < target)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }
    return -1;
}

int binary_search_raw(int* data, int size, int target, int &value){
    int left = 0;
    int right = size;
    while (left <= right)
    {
        int mid = left + (right - left) / 2;

        int key = data[mid * 2];

        if (key == target)
        {
            value = data[mid * 2 + 1];
            return mid;
        }
        else if (key < target)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }
    return -1;
}