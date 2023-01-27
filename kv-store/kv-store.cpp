// kv-store.cpp : Defines the entry point for the application.
//

#include "kv-store.h"
#include "red-black-tree.h"

using namespace std;

int main()
{
	cout << "Hello CMake. " << sizeof(long) << endl;
    RedBlackTree *memtable = new RedBlackTree(0, 0);
    memtable->insert(1, 5);
    memtable->insert(2, 4);
    memtable->insert(-1, 5);
    std::tuple<int *, int> tup = memtable->scan(-20, 20);
    int * arr = get<0>(tup);
    int size = get<1>(tup);
    cout << "arr size: " << size << endl;
    for (int i = 0; i < size; i++) {
        cout << arr[i] << ",";
    }
    cout << endl << memtable->get(1) << memtable->get(2) << memtable->get(-1) << memtable->get(5) << endl;
    return 0;
}
