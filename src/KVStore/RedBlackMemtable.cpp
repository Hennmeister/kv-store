#include "../../include/RedBlack/RedBlackMemtable.h"
#include "../../include/RedBlack/RedBlackTree.h"


using namespace std;
RedBlackMemtable::RedBlackMemtable()
{
    this->data = new RedBlackTree();
}

int RedBlackMemtable::get_size()
{
    return this->data->getSize();
}

bool RedBlackMemtable::put(const int &key, const int &value)
{
    data->put(key, value);
    return true;
}

bool RedBlackMemtable::get(const int &key, int &value)
{
    return data->get(key, value);
}

vector<pair<int, int>> RedBlackMemtable::scan(const int &key1, const int &key2)
{
    return data->scan(key1, key2);
}

vector<pair<int, int>> RedBlackMemtable::inorderTraversal()
{
    return data->inorderTraversal();
}

bool RedBlackMemtable::reset()
{
    // TODO: Free previous redblacktree if necessary
    this->data = new RedBlackTree();
    return true;
}

RedBlackMemtable::~RedBlackMemtable()
{
}
