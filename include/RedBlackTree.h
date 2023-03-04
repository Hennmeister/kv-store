
#ifndef KV_STORE_REDBLACKTREE_H
#define KV_STORE_REDBLACKTREE_H

#include <tuple>
#include <climits>

enum Color {RED, BLACK};

class Node {
public:
    int key;
    int value;
    bool color;
    Node* left, * right, * parent;

    explicit Node(int, int);
};

class RedBlackTree {
private:
    Node* root;
    Node* nil;
    int min_key{};
    int max_key{};
    int size;

    void rotateLeft(Node*&, Node*&);
    void rotateRight(Node*&, Node*&);
    void fixViolation(Node*&, Node*&);
    void inorderTraversalHelper(Node* root, const int& key1, const int& key2, std::vector<std::pair<int, int>> &kv_pairs);

public:
    explicit RedBlackTree();

    void put(const int &key, const int &value);
    bool get(const int& key, int& value);
    std::vector<std::pair<int, int>> scan(const int& key1, const int& key2);
    std::vector<std::pair<int, int>> inorderTraversal();
    int getSize();
};

#endif //KV_STORE_REDBLACKTREE_H
