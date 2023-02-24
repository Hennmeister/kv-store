#ifndef KV_STORE_SIMPLESSTMANAGER_H
#define KV_STORE_SIMPLESSTMANAGER_H

#include <string>
#include <fstream>
#include "Base/SSTManager.h"

class SimpleSSTManager: public SSTManager{
private:
    std::string directory;
    std::pair<int,int> read_entry_from_back(std::ifstream *file, int offset, bool ignore_val);
    std::pair<int,int> read_entry_at_offset(std::ifstream *file, int offset, bool ignore_val);
    int index_to_offset(int start, int index);
    bool binary_search(int start, int end, int target, int &value);
public:
    explicit SimpleSSTManager(std::string target_dir);
    bool get(const int& key, int &value) override;
    std::vector<std::pair<int, int>> scan(const int& key1, const int& key2) override;
    bool add_sst(std::vector<std::pair<int, int>> data) override;
};
#endif //KV_STORE_SIMPLESSTMANAGER_H
