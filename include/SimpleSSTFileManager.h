#ifndef KV_STORE_SIMPLESSTFILEMANAGER_H
#define KV_STORE_SIMPLESSTFILEMANAGER_H

#include <string>
#include "Base/SSTFileManager.h"
#include "Base/BufferPool.h"

using namespace std;
class SimpleSSTFileManager : public SSTFileManager {
private:
    string dir_name;
    vector<int> files;
    BufferPool *cache;
public:
    explicit SimpleSSTFileManager(string target_dir, BufferPool *cache);
    ~SimpleSSTFileManager();
    int get_page(int page, string filename, void* data_buf);
    int scan(int start_page, int end_page, string filename, void* data_buf);
    int write_file(void* data, int size, string filename, void* metadata);
    vector<pair<string, int>> get_files();
    int get_metadata(void* data, string filename) override;
};

#endif //KV_STORE_SIMPLESSTFILEMANAGER_H
