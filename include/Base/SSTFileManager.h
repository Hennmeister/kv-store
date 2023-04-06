#ifndef KV_STORE_SSTFILEMANAGER_H
#define KV_STORE_SSTFILEMANAGER_H
#include <vector>
#include <string>

using namespace std;
class SSTFileManager {
public:
    // Load a single page from the file
    virtual int get_page(int page, string filename, void* data_buf) =0;

    // Read a set of pages from the file
    virtual int scan(int start_page, int end_page, string filename, void* data_buf) =0;

    // Write out a buffer of data to a file - Metadata must be assigned, if unused pass in empty PAGE_SIZE bytes.
    virtual int write_file(void* data, int size, string filename, void* metadata)=0;

    // Get a list of files and their sizes (in bytes) in the directory
    virtual vector<pair<string, int>> get_files()=0;

    // Get 1st page (metadata)
    virtual int get_metadata(void* data, string filename)=0;
};

#endif //KV_STORE_SSTFILEMANAGER_H
