
#ifndef KV_STORE_RED_BLACK_TREE_H
#define KV_STORE_RED_BLACK_TREE_H

#include <tuple>
#include "../kv-store/memtable.h"

enum Color {RED, BLACK};

class Node {
public:
    int key;
    int value;
    bool color;
    Node* left, * right, * parent;

    explicit Node(int, int);
};

class RedBlackTree : public Memtable {
private:
    Node* root;
    Node* nil;
    int min_key{};
    int max_key{};
    int capacity;
    int size;
    std::string directory;

    void rotateLeft(Node*&, Node*&);
    void rotateRight(Node*&, Node*&);
    void fixViolation(Node*&, Node*&);

public:
    explicit RedBlackTree(const int &memtable_size, const std::string &directory);

    void put(const int& key, const int& value) override;
    int get(const int& key) override;
    std::vector<std::pair<int, int>> scan(const int& key1, const int& key2) override;
    std::vector<std::pair<int, int>> inorderTraversal() override;
};

#endif //KV_STORE_RED_BLACK_TREE_H
