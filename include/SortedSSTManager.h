#ifndef KV_STORE_SORTEDSSTMANAGER_H
#define KV_STORE_SORTEDSSTMANAGER_H

#include <string>
#include <fstream>
#include "Base/SSTManager.h"

class SortedSSTManager: public SSTManager{
private:
    std::string directory;
    int index_fd;
    int sst_fd;
    std::vector<int> sizes;
    int total_entries;
    std::vector<std::pair<int, int>> get_sst(int sst_ind);
public:
    ~SortedSSTManager();
    explicit SortedSSTManager(std::string prefix);
    bool get(const int& key, int &value) override;
    std::vector<std::pair<int, int>> scan(const int& key1, const int& key2) override;
    bool add_sst(std::vector<std::pair<int, int>> data) override;
    void delete_data() override;
};
#endif //KV_STORE_SORTEDSSTMANAGER_H
