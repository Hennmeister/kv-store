#include "../include/SimpleSSTFileManager.h"
#include "../include/Base/BufferPool.h"
#include <filesystem>
#include <fcntl.h>
#include "../include/util.h"
#include <unistd.h>
#include "../include/constants.h"
#include <sys/stat.h>
#include <string>
#include <iostream>
#include <filesystem>
#include <stdint.h>

long GetFileSize(std::string filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}


int get_ind_from_fname(string fname){
    string delimiter = ".";
    string token = fname.substr(0, fname.find(delimiter));
    return stoi(token);
}



SimpleSSTFileManager::SimpleSSTFileManager(std::string target_dir, BufferPool *cache) {
    this->cache = cache;
    this->dir_name = target_dir;
    int dir = dir_exists(target_dir);

    if (dir == 0)
    {
        filesystem::create_directory(target_dir);
    }
    else if (dir != 1)
    {
        throw invalid_argument("Provided path is a file: unable to initialize database");
    }
    int max_file_ind = 0;
    this->files = vector<int>();

    auto file_list =  vector<string>();
    for (const auto &entry : std::filesystem::directory_iterator(dir_name))
        file_list.emplace_back(entry.path().filename());

    for(const string& file: file_list){
        max_file_ind = max(get_ind_from_fname(file), max_file_ind);
    }

    for(int i = 0; i <= max_file_ind; i++){
        files.push_back(-1);
    }

    for(const string& file: file_list){
        files[get_ind_from_fname(file)] = open((dir_name + "/" + file).c_str(), O_RDWR, 0777);
    }

}

SimpleSSTFileManager::~SimpleSSTFileManager() {
    for(int file_fd: this->files){
        if(file_fd != -1){
            close(file_fd);
        }
    }
}

int SimpleSSTFileManager::get_page(int page, string file, void *data_buf) {
    // Account for metadata_page
    page++;

    string buf_entry_id = file + "-" + to_string(page);

    // Check if entry_data is cached
    if (this->cache->get(buf_entry_id, (std::uint8_t *) data_buf)) {
        return PAGE_SIZE;
    }

    int file_fd = this->files[get_ind_from_fname(file)];
    int successful_read = safe_read(file_fd, data_buf, PAGE_SIZE, PAGE_SIZE * page);
//    close(file_fd);

    // Add page to cache
    this->cache->put(buf_entry_id, (std::uint8_t *) data_buf, PAGE_SIZE);
    return successful_read;
}

// Reads in end entry_data as well (inclusive) e.g. start = 0 end = 0 implies only 0 is read, start = 0 end = 1 implies 0,1 read
int SimpleSSTFileManager::scan(int start_page, int end_page, string file, void *data_buf, bool should_cache) {
    // Account for metadata entry_data.
    start_page++;
    end_page++;

    int diff = (end_page + 1) - start_page;

    // check if there is a variable sized entry
    // this is used for bloom filter entries
    string buf_entry_id = file + "-" + to_string(start_page) + "-" + to_string(end_page);
    if (should_cache &&  this->cache->get(buf_entry_id, (std::uint8_t *) data_buf)) {
       return diff * PAGE_SIZE;
    }

    // Don't cache scanned pages to avoid sequential flooding
    int file_fd = this->files[get_ind_from_fname(file)];
    int successful_read = safe_read(file_fd, data_buf, PAGE_SIZE * diff, PAGE_SIZE * start_page);
//    close(file_fd);

    if (should_cache) {
        this->cache->put(buf_entry_id, (std::uint8_t *) data_buf, diff * PAGE_SIZE);
    }
    return successful_read;
}

int SimpleSSTFileManager::write_file(void *data, int size, string new_filename, void* metadata) {
    string filename = dir_name + "/" + new_filename;
    int file_fd = open(filename.c_str(), O_RDWR | O_CREAT, 0777);
    int file_ind = get_ind_from_fname(new_filename);
    while(file_ind >= this->files.size()){
        this->files.push_back(-1);
    }
    this->files[file_ind] = file_fd;
    int meta_write = safe_write(file_fd, metadata, PAGE_SIZE, 0);
    int successful_write = safe_write(file_fd, data, size, PAGE_SIZE);
    return successful_write + meta_write;
}

vector<pair<string, int>> SimpleSSTFileManager::get_files() {
    auto files =  vector<pair<string, int>>();
    for (const auto &entry : std::filesystem::directory_iterator(dir_name))
        files.emplace_back(entry.path().filename(), GetFileSize(entry.path()) );
    return files;
}

int SimpleSSTFileManager::get_metadata(void *data, string filename) {
    return this->get_page(-1, filename, data);
}

bool SimpleSSTFileManager::delete_file(string filename) {
    int ind = get_ind_from_fname(filename);
    if(this->files[ind] != -1){
        close(this->files[ind]);
    }
    if(remove((dir_name + "/" + filename).c_str()) == -1) {
        perror("Error deleting file");
        return false;
    }
    return true;
}

int SimpleSSTFileManager::write_page(void *data, int size, int start_page_num, string fname) {
//    string filename = dir_name + "/" + fname;
    int file_fd = this->files[get_ind_from_fname(fname)];
//    int file_fd = open(filename.c_str(), O_RDWR, 0777);
    int successful_write = safe_write(file_fd, data, size, start_page_num * PAGE_SIZE);
//    close(file_fd);
    return successful_write;
}

