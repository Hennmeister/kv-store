#ifndef KV_STORE_SSTFILEMANAGER_H
#define KV_STORE_SSTFILEMANAGER_H
#include <vector>
class SSTFileManager {
public:
    virtual int get_page(int page, int file, void* data_buf) =0;
    virtual int scan(int start_page, int end_page, int file, void* data_buf) =0;
    virtual int write_file(void* data, int size, int &new_file_num)=0;
};

#endif //KV_STORE_SSTFILEMANAGER_H
