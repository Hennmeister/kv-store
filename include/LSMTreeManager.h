#ifndef KV_STORE_LSMTREEMANAGER_H
#define KV_STORE_LSMTREEMANAGER_H
#include "./Base/SSTManager.h"
#include "./Base/SSTFileManager.h"
#include "./BTree/BTreeSST.h"

using namespace std;
class LSMTreeManager: public SSTManager{
private:
    SSTFileManager *fileManager;
    vector<vector<BTreeSST*>> levels;
    int sst_counter;
    int total_entries;
    int useBinary;
    int newFanout;
    int filter_bits_per_entry;
    int memtable_size;
    bool compact_tree(int level);
    BTreeSST* combine_SST(BTreeSST* newer, BTreeSST* older);
public:
    ~LSMTreeManager();
    explicit LSMTreeManager(SSTFileManager *fileManager, int newFanout, int useBinarySearch, int memtable_size, int filter_bits_per_entry);
    bool get(const int& key, int &value) override;
    std::vector<std::pair<int, int>> scan(const int& key1, const int& key2) override;
    bool add_sst(std::vector<std::pair<int, int>> data) override;
};
#endif //KV_STORE_LSMTREEMANAGER_H
