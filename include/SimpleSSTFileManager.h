#ifndef KV_STORE_SIMPLESSTFILEMANAGER_H
#define KV_STORE_SIMPLESSTFILEMANAGER_H

#include <string>
#include "Base/SSTFileManager.h"

using namespace std;
class SimpleSSTFileManager : public SSTFileManager {
private:
    string dir_name;
    vector<int> files;
public:
    explicit SimpleSSTFileManager(string target_dir);
    ~SimpleSSTFileManager();
    virtual int get_page(int page, string filename, void* data_buf);
    virtual int scan(int start_page, int end_page, string filename, void* data_buf);
    virtual int write_file(void* data, int size, string filename, void* metadata);
    virtual vector<pair<string, int>> get_files();
    virtual int get_metadata(void* data, string filename);
};

#endif //KV_STORE_SIMPLESSTFILEMANAGER_H
