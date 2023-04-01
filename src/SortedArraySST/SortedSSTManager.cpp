#include "../../include/ArraySST/SortedSSTManager.h"
#include "../../include/util.h"
#include "../../include/constants.h"
#include "../../include/util.h"

using namespace std;

int binary_search(vector<pair<int, int>> data, int target, int &value)
{
    int left = 0;
    int right = data.size();
    while (left <= right)
    {
        int mid = left + (right - left) / 2;

        int key = data[mid].first;

        if (key == target)
        {
            value = data[mid].second;
            return mid;
        }
        else if (key < target)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }
    return -1;
}

std::vector<std::pair<int, int>> single_sst_scan(std::vector<std::pair<int, int>> sst_dat, int start, int end)
{
    auto res = vector<pair<int, int>>();

    // Find the first element greater than or equal to the start index.
    auto elem = lower_bound(sst_dat.begin(), sst_dat.end(), start,
                            [](const pair<int, int> &info, double value)
                            {
                                return info.first < value;
                            });

    if (elem == sst_dat.end())
    {
        return res;
    }
    int ind = distance(sst_dat.begin(), elem);

    while (ind < sst_dat.size() && sst_dat[ind].first <= end)
    {
        res.emplace_back(sst_dat[ind]);
        ind++;
    }
    return res;
}

bool SortedSSTManager::get(const int &key, int &value)
{
    for (int i = sst_count - 1; i > -1; i--)
    {
        if (binary_search(get_sst(i), key, value) != -1)
        {
            return true;
        }
    }
    return false;
}

bool SortedSSTManager::add_sst(vector<pair<int, int>> data)
{
    int sz = data.size();
    if (sz % PAGE_NUM_ENTRIES == 0)
        return false;

    int *write_buf = new int[sz * 2];
    for (int i = 0; i < sz; i++)
    {
        write_buf[i * 2] = data[i].first;
        write_buf[i * 2 + 1] = data[i].second;
    }


    string fname = to_string(sst_count + 1) + ".sst";
    int *meta = new int[PAGE_SIZE/sizeof(int)];
    fileManager->write_file(write_buf, sz * ENTRY_SIZE, fname, meta);

    delete[] write_buf;
    delete[] meta;

    sst_count += 1;
    total_entries += sz;
    //    print_data(this->get_sst(sizes.size() - 1)); - Can be used to debug writes
    return true;
}

vector<pair<int, int>> SortedSSTManager::scan(const int &key1, const int &key2)
{
    auto res = vector<pair<int, int>>();

    for (int i = sst_count - 1; i > -1; i--)
    {
        res = priority_merge(res, single_sst_scan(get_sst(i), key1, key2));
    }
    return res;
}

SortedSSTManager::SortedSSTManager(SSTFileManager *fileManager)
{
    this->fileManager = fileManager;
    sst_count = fileManager->get_files().size();
}

SortedSSTManager::~SortedSSTManager()
{
}

bool sortByFname(const pair<string,int> &a,
               const pair<string,int> &b)
{
    int pos_a = stoi(a.first.substr(0, a.first.size()-4));
    int pos_b = stoi(b.first.substr(0, b.first.size()-4));
    return (pos_a < pos_b);
}


vector<pair<string, int>> SortedSSTManager::get_ssts(){
    auto files = fileManager->get_files();
    std::sort(files.begin(), files.end(), sortByFname);
    return files;
}

vector<pair<int, int>> SortedSSTManager::get_sst(int sst_ind)
{
    auto res = vector<pair<int, int>>();
    if(sst_ind >= sst_count){
        return res;
    }
    auto files = this->get_ssts();

    int total_read = files[sst_ind].second - PAGE_SIZE;



    int *data = new int[total_read/sizeof(int)];

    fileManager->scan(0, total_read/PAGE_SIZE, files[sst_ind].first, data);

    for (int ind = 0; ind < total_read/ENTRY_SIZE; ind++)
    {
        res.emplace_back(data[ind * 2], data[ind * 2 + 1]);
    }

    delete[] data;
    return res;
}
