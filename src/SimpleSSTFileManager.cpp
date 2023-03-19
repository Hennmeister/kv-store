#include "../include/SimpleSSTFileManager.h"
#include <filesystem>
#include <fcntl.h>
#include "../include/util.h"
#include <unistd.h>
#include "../include/constants.h"
#include <sys/stat.h>
#include <string>
#include <iostream>
#include <filesystem>

long GetFileSize(std::string filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

SimpleSSTFileManager::SimpleSSTFileManager(std::string target_dir) {
    dir_name = target_dir;
    int dir = dir_exists(target_dir);

    if (dir == 0)
    {
        filesystem::create_directory(target_dir);
    }
    else if (dir != 1)
    {
        throw invalid_argument("Provided path is a file: unable to initialize database");
    }
}

SimpleSSTFileManager::~SimpleSSTFileManager() {

}

int SimpleSSTFileManager::get_page(int page, string file, void *data_buf) {
    char* filename = string_to_char(dir_name + "/" + file);
    int file_fd = open(filename, O_RDWR, 0777);
    int successful_read = safe_read(file_fd, data_buf, PAGE_SIZE, PAGE_SIZE * page);
    close(file_fd);
    return successful_read;
}

int SimpleSSTFileManager::scan(int start_page, int end_page, string file, void *data_buf) {
    char* filename = string_to_char(dir_name + "/" + file);
    int file_fd = open(filename, O_RDWR, 0777);
    int diff = end_page - start_page;
    int successful_read = safe_read(file_fd, data_buf, PAGE_SIZE * diff, PAGE_SIZE * start_page);
    close(file_fd);
    return successful_read;
}

int SimpleSSTFileManager::write_file(void *data, int size, string new_filename) {
    char* filename = string_to_char(dir_name + "/" + new_filename);
    int file_fd = open(filename, O_RDWR | O_CREAT, 0777);
    int successful_write = safe_write(file_fd, data, size,0);
    close(file_fd);
    return successful_write;
}

vector<pair<string, int>> SimpleSSTFileManager::get_files() {
    auto files =  vector<pair<string, int>>();
    for (const auto &entry : std::filesystem::directory_iterator(dir_name))
        files.emplace_back(entry.path().filename(), GetFileSize(entry.path())/PAGE_SIZE );
    return files;
}


