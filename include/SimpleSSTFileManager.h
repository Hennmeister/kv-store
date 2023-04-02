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
    int get_page(int page, string filename, void* data_buf) override;
    int scan(int start_page, int end_page, string filename, void* data_buf) override;
    int write_file(void* data, int size, string filename, void* metadata) override;
    vector<pair<string, int>> get_files() override;
    int get_metadata(void* data, string filename) override;
};

#endif //KV_STORE_SIMPLESSTFILEMANAGER_H