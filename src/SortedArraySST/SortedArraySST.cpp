#include <vector>
#include "../../include/SortedArraySST.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

bool SortedArraySST::get(const int &key, int& value) {
    // TODO: binary search?
    for (std::pair<int,int> pair : data) {
        if(pair.first == key){
            value = pair.second;
            return true;
        }
    }
    return false;
}

vector<std::pair<int, int>> SortedArraySST::scan(const int &key1, const int &key2) {
    std::vector<std::pair<int, int>> vec;
    return vec;
}

SortedArraySST::SortedArraySST(const filesystem::directory_entry file) {
    // TODO: Update SSTManager and SST class with single file structure.
    std::ifstream read_file(file.path());
    std::string text;
    std::getline(read_file,text);

    size = std::stoi(text);
    this->data = {};
    for (int i =0; i < size;i++) {
        std::getline(read_file,text);
        int k = std::stoi(text.substr(0, text.find(",")));
        int v = std::stoi(text.substr(text.find(",") + 1, text.size()));
        data.emplace_back(k, v);
    }
    read_file.close();
}

SortedArraySST::SortedArraySST(std::string target_dir, const vector<std::pair<int, int>>& data) {
    ofstream *sst_file = new ofstream();
    // TODO: remove .txt once visualization is not needed
    sst_file->open(target_dir + "/ssts.txt", ios::binary | ios::app);

    if (!sst_file->is_open())
        throw std::runtime_error("SST data file could not be opened");

    // Write key,val pairs to sst file
    for (pair<int,int> pair : data)
        (* sst_file) << pair.first << "," << pair.second << ";";

    sst_file->close();

    if (!filesystem::exists(target_dir + "/index.txt")) {
        ofstream *write_index_file = new ofstream();
        write_index_file->open(target_dir + "/index.txt", ios::binary | ios::app);

        if (!write_index_file->is_open())
            throw std::runtime_error("SST data file could not be opened");

        // This is the first SST to be saved, so offset in SST file is 0
        string entry = "0," + to_string(data.size());

        cout << entry << endl;

        (*write_index_file) << entry.length() << ";";
        (*write_index_file) << entry << ";";

        write_index_file->close();
    } else {
        ifstream *read_index_file = new ifstream();
        read_index_file->open(target_dir + "/index.txt", ios::in | ios::binary);

        if (!read_index_file->is_open())
            throw std::runtime_error("SST data file could not be opened");

        string last_entry_size;
        getline(*read_index_file, last_entry_size);

        cout << last_entry_size << endl;

        // Seek to last line of the file
        read_index_file->seekg(-stoi(last_entry_size) - 1, ios_base::end);

        // Calculate sst offset from index file
        string prev_offset;
        string prev_size;

        getline(*read_index_file, prev_offset, ','); // SST's offset
        getline(*read_index_file, prev_size, ';'); // Read SST's size

        read_index_file->close();

        cout << prev_offset << "," << prev_size << ";";

        int offset = stoi(prev_offset) + stoi(prev_size);

        ofstream *write_index_file = new ofstream();
        write_index_file->open(target_dir + "/index.txt", ios::binary | ios::app);

        if (!write_index_file->is_open())
            throw std::runtime_error("Could not write to SST index file");

        string entry = to_string(offset) + "," + to_string(data.size());
        (*write_index_file) << entry << ";";

        write_index_file->close();

        if (stoi(last_entry_size) != entry.length()) {
            // TODO: this should rarely ever happen (only due to internal modifications of memtable capacity
            // But there are a few options here:

            // do the following:
            // 1) Create the new file.
            // 2) Copy the old file to the new file up to the start of the "line" you want to change
            // 3) Write the "new" line to the new file.
            // 4) Skip the "old" line in the old file.
            // 5) Copy the remainder of the old file to the new file.
            // 6) Close both old and new files.
            // 7) Delete the old file, and rename the new file.

            // OR

            // have a trivial file store last_entry_size by itself (pretty efficient, but this adds another file descriptor to handle)
        }
    }

}

