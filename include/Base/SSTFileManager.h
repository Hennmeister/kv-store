#ifndef KV_STORE_SSTFILEMANAGER_H
#define KV_STORE_SSTFILEMANAGER_H
#include <vector>
using namespace std;
class SSTFileManager {
public:
    virtual int get_page(int page, string filename, void* data_buf) =0;
    virtual int scan(int start_page, int end_page, string filename, void* data_buf) =0;
    virtual int write_file(void* data, int size, string filename)=0;
    virtual vector<pair<string, int>> get_files()=0;
};

#endif //KV_STORE_SSTFILEMANAGER_H
