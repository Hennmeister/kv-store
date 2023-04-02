
#include <fcntl.h>
#include "../../include/SortedSSTManager.h"
#include "../../include/util.h"
#include "../../include/constants.h"
#include "../../include/util.h"
#include <unistd.h>
#include <sys/stat.h>
#include <filesystem>
#include <iostream>

using namespace std;

int binary_search(vector<pair<int, int>> data, int target, int &value)
{
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

std::vector<std::pair<int, int>> single_sst_scan(std::vector<std::pair<int, int>> sst_dat, int start, int end)
{
    auto res = vector<pair<int, int>>();

    // Find the first element greater than or equal to the start index.
    auto elem = lower_bound(sst_dat.begin(), sst_dat.end(), start,
                            [](const pair<int, int> &info, double value)
                            {
                                return info.first < value;
                            });

    if (elem == sst_dat.end())
    {
        return res;
    }
    int ind = distance(sst_dat.begin(), elem);

    while (ind < sst_dat.size() && sst_dat[ind].first <= end)
    {
        res.emplace_back(sst_dat[ind]);
        ind++;
    }
    return res;
}

bool SortedSSTManager::get(const int &key, int &value)
{
    for (int i = sizes.size() - 1; i > -1; i--)
    {
        if (binary_search(get_sst(i), key, value) != -1)
        {
            return true;
        }
    }
    return false;
}

bool SortedSSTManager::add_sst(vector<pair<int, int>> data)
{
    int sz = data.size();
    if (sz % PAGE_NUM_ENTRIES == 0)
        return false;

    int *write_buf = new int[sz * 2];
    for (int i = 0; i < sz; i++)
    {
        write_buf[i * 2] = data[i].first;
        write_buf[i * 2 + 1] = data[i].second;
    }

    safe_write(sst_fd, write_buf, sz * ENTRY_SIZE,
               total_entries * ENTRY_SIZE);

    delete[] write_buf;

    safe_write(index_fd, &sz, sizeof(int),
               sizes.size() * sizeof(int));

    sizes.emplace_back(sz);
    total_entries += sz;
    //    print_data(this->get_sst(sizes.size() - 1)); - Can be used to debug writes
    return true;
}

vector<pair<int, int>> SortedSSTManager::scan(const int &key1, const int &key2)
{
    auto res = vector<pair<int, int>>();

    for (int i = sizes.size() - 1; i > -1; i--)
    {
        res = priority_merge(res, single_sst_scan(get_sst(i), key1, key2));
    }
    return res;
}

SortedSSTManager::SortedSSTManager(string prefix)
{
    this->directory = prefix;
    char *index_file = string_to_char(prefix + "/index.sdb");
    char *sst_file = string_to_char(prefix + "/sst.sdb");

    int dir = dir_exists(prefix);

    if (dir == 0)
    {
        filesystem::create_directory(prefix);
    }
    else if (dir != 1)
    {
        throw invalid_argument("Provided path is a file: unable to initialize database");
    }

    // TODO: O_DIRECT DOES NOT exist on Mac os or windows - Perhaps someone can try using O_DIRECT on linux?
    index_fd = open(index_file, O_RDWR | O_CREAT, 0777);
    sst_fd = open(sst_file, O_RDWR | O_CREAT, 0777);

    delete[] index_file;
    delete[] sst_file;

    if (index_fd == -1 || sst_fd == -1)
    {
        ::perror("Open Failed");
    }

    sizes = vector<int>();

    // Read first line of index file
    int read_buf;
    int byte_read_count = pread(index_fd, &read_buf, sizeof(int), 0);
    int offset = byte_read_count;
    total_entries = 0;

    // Populate the sizes vector with sizes
    while (byte_read_count > 0)
    {
        sizes.push_back(read_buf);
        total_entries += read_buf;
        byte_read_count = pread(index_fd, &read_buf, sizeof(int), offset);
        offset += byte_read_count;
    }
}

SortedSSTManager::~SortedSSTManager()
{
    close(index_fd);
    close(sst_fd);
}

vector<pair<int, int>> SortedSSTManager::get_sst(int sst_ind)
{
    auto res = vector<pair<int, int>>();

    int start_offset = 0;
    int total_read = sizes[sst_ind] * ENTRY_SIZE;

    for (int i = 0; i < sst_ind; i++)
    {
        start_offset += sizes[i];
    }
    start_offset *= ENTRY_SIZE;

    int *data = new int[sizes[sst_ind] * 2];
    safe_read(sst_fd, data, total_read, start_offset);

    for (int ind = 0; ind < sizes[sst_ind]; ind++)
    {
        res.emplace_back(data[ind * 2], data[ind * 2 + 1]);
    }

    delete[] data;
    return res;
}

void SortedSSTManager::delete_data() {
    std::error_code errorCode;
    if (!filesystem::remove_all(this->directory, errorCode)) {
        throw std::runtime_error("Error deleting data: " + errorCode.message());
    }
}
