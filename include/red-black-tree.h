
#ifndef KV_STORE_RED_BLACK_TREE_H
#define KV_STORE_RED_BLACK_TREE_H

#include <tuple>
#include "memtable-data.h"

enum Color {RED, BLACK};

class Node {
public:
    int key;
    int value;
    bool color;
    Node* left, * right, * parent;

    explicit Node(int, int);
};

class RedBlackTree : public MemtableData {
private:
    Node* root;
    Node* nil;
    int min_key{};
    int max_key{};
    int size;

    void rotateLeft(Node*&, Node*&);
    void rotateRight(Node*&, Node*&);
    void fixViolation(Node*&, Node*&);

public:
    explicit RedBlackTree();

    void put(const int& key, const int& value) override;
    int get(const int& key) override;
    std::vector<std::pair<int, int>> scan(const int& key1, const int& key2) override;
    std::vector<std::pair<int, int>> inorderTraversal() override;
    int getSize() override;
};

#endif //KV_STORE_RED_BLACK_TREE_H
