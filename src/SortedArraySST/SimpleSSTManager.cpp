#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include "../../include/SimpleSSTManager.h"
#include "../../include/constants.h"
#include "../../include/util.h"
#include <sys/stat.h>
#include <filesystem>
#include <cstring>

using namespace std;

SimpleSSTManager::SimpleSSTManager(std::string target_dir)
{
    this->directory = target_dir;

    int dir = dir_exists(target_dir);
    if (dir == 0)
    {
        mkdir(target_dir.c_str(), 0777);
    }
    else if (dir == 1)
    {
        // clear directory from previous data
        for (const auto &entry : std::filesystem::directory_iterator(target_dir))
            std::filesystem::remove_all(entry.path());
    }
    else
    {
        throw std::invalid_argument("Provided path is a file: unable to initialize database");
    }
}

pair<int, int> SimpleSSTManager::read_entry_from_back(ifstream *file, int offset, bool ignore_val = false)
{
    if (!file->is_open())
        throw std::runtime_error("File could not be opened when reading from the back at offset " + to_string(offset));

    // Seek to <offset> from the end of the file
    file->seekg(-offset, ios_base::end);

    int32_t first, second;
    file->read(reinterpret_cast<char *>(&first), sizeof(int32_t));
    if (!ignore_val)
        file->read(reinterpret_cast<char *>(&second), sizeof(int32_t));

    return std::make_pair(first, second);
}

pair<int, int> SimpleSSTManager::read_entry_at_offset(ifstream *file, int offset, bool ignore_val = false)
{
    if (!file->is_open())
        throw std::runtime_error("File could not be opened when reading at offset " + to_string(offset));

    file->seekg(offset);

    int32_t first, second;
    file->read(reinterpret_cast<char *>(&first), sizeof(int32_t));
    if (!ignore_val)
        file->read(reinterpret_cast<char *>(&second), sizeof(int32_t));

    return std::make_pair(first, second);
}

/**
 * This function assumes data is page aligned and thus have
 * a size proportional to PAGE_NUM_ENTRIES
 */
bool SimpleSSTManager::add_sst(vector<pair<int32_t, int32_t>> data)
{

    if (data.size() % PAGE_NUM_ENTRIES == 0)
        return false;

    ofstream *sst_file = new ofstream();

    sst_file->open(this->directory + "/ssts", ios::binary | ios::app);

    if (!sst_file->is_open())
        throw std::runtime_error("SST data file could not be opened");

    char write_data[data.size() * ENTRY_SIZE];

    // Load array to write
    for (int i = 0; i < data.size(); i++)
    {
        memcpy(&write_data[i * ENTRY_SIZE], &data[i].first, sizeof(int32_t));
        memcpy(&write_data[i * ENTRY_SIZE + sizeof(int32_t)], &data[i].second, sizeof(int32_t));
    }

    // Write key,val pairs to sst file
    sst_file->write(write_data, data.size() * ENTRY_SIZE);
    sst_file->close();

    if (!filesystem::exists(this->directory + "/index"))
    {
        ofstream *write_index_file = new ofstream();
        write_index_file->open(this->directory + "/index", ios::binary | ios::app);

        if (!write_index_file->is_open())
            throw std::runtime_error("SST data file could not be opened");

        // This is the first SST to be saved, so offset in SST file is 0
        int32_t zero = 0;
        int32_t data_size = data.size() * ENTRY_SIZE;
        write_index_file->write(reinterpret_cast<const char *>(&zero), sizeof(int32_t));
        write_index_file->write(reinterpret_cast<const char *>(&data_size), sizeof(int32_t));

        write_index_file->close();
    }
    else
    {
        ifstream *read_index_file = new ifstream();
        read_index_file->open(this->directory + "/index", ios::in | ios::binary);

        pair<int, int> entry = read_entry_from_back(read_index_file, ENTRY_SIZE);
        // Read index's offset and size
        int32_t new_offset = entry.first + entry.second;

        read_index_file->close();

        ofstream *write_index_file = new ofstream();
        write_index_file->open(this->directory + "/index", ios::binary | ios::app);

        if (!write_index_file->is_open())
            throw std::runtime_error("Could not write to SST index file");

        write_index_file->write(reinterpret_cast<char *>(&new_offset), sizeof(int32_t));
        int32_t data_size = data.size() * ENTRY_SIZE;
        write_index_file->write(reinterpret_cast<char *>(&data_size), sizeof(int32_t));

        write_index_file->close();
    }

    return true;
}

std::vector<std::pair<int, int>> SimpleSSTManager::scan(const int &key1, const int &key2)
{
    std::vector<std::pair<int, int>> vec;
    return vec;
}

int SimpleSSTManager::index_to_offset(int index)
{
    return index * ENTRY_SIZE;
}

bool SimpleSSTManager::binary_search(char *entries, int capacity, int target, int &value)
{
    int left = 0;
    int right = capacity / ENTRY_SIZE - 1;

    while (left <= right)
    {
        int mid = left + (right - left) / 2;

        int key;
        memcpy(&key, &entries[index_to_offset(mid)], sizeof(int32_t));

        if (key == target)
        {
            memcpy(&value, &entries[index_to_offset(mid)] + sizeof(int32_t), sizeof(int32_t));
            return true;
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
    return false;
}

bool SimpleSSTManager::get(const int &key, int &value)
{
    if (!filesystem::exists(this->directory + "/index"))
        return false;

    ifstream *read_index_file = new ifstream();
    read_index_file->open(this->directory + "/index", ios::in | ios::binary);

    if (!read_index_file->is_open())
        throw std::runtime_error("Could not read SST index file");

    int begin = read_index_file->tellg();
    read_index_file->seekg(0, ios::end);
    int end = read_index_file->tellg();
    int file_size = end - begin;

    bool found = false;
    int offset = ENTRY_SIZE;
    pair<int, int> entry;
    int32_t read;
    while (!found && offset <= file_size)
    {
        entry = read_entry_from_back(read_index_file, offset);

        ifstream *read_ssts_file = new ifstream();
        read_ssts_file->open(this->directory + "/ssts", ios::in | ios::binary);

        if (!read_ssts_file->is_open())
            throw std::runtime_error("Could not read SST data file");

        // Seek to sst offset
        read_ssts_file->seekg(entry.first);

        char sst[entry.second];
        read_ssts_file->read(sst, entry.second);
        read_ssts_file->close();

        found = binary_search(sst, entry.second, key, read);
        offset += ENTRY_SIZE;
    }

    read_index_file->close();

    if (!found)
        return false;

    value = read;
    return true;
}

SimpleSSTManager::~SimpleSSTManager()
{
}
