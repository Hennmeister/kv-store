#ifndef KV_STORE_SORTEDSSTFILEMANAGER_H
#define KV_STORE_SORTEDSSTFILEMANAGER_H
#include "Base/SSTFileManager.h"

class SortedSSTFileManager : public SSTFileManager {
public:
    explicit SortedSSTFileManager(std::string target_dir);
    ~SortedSSTFileManager();
    virtual int get_page(int page, int file, void* data_buf);
    virtual int scan(int start_page, int end_page, int file, void* data_buf);
    virtual int write_file(void* data, int size, int &new_file_num);
};

#endif //KV_STORE_SORTEDSSTFILEMANAGER_H
