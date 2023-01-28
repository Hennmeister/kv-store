//
// Created by Henning Lindig on 2023-01-26.
//

#ifndef KV_STORE_RED_BLACK_TREE_H
#define KV_STORE_RED_BLACK_TREE_H

#include <tuple>

enum Color {RED, BLACK};

class Node {
public:
    int key;
    int value;
    bool color;
    Node* left, * right, * parent;

    explicit Node(int, int);
};

class RedBlackTree{
private:
    Node* root;
    Node* nil;

    void rotateLeft(Node*&, Node*&);
    void rotateRight(Node*&, Node*&);
    void fixViolation(Node*&, Node*&);

public:
    int min_key;
    int max_key;

    RedBlackTree(const int& key, const int& value);

    void insert(const int& key, const int& value);
    int get(const int& key);
    std::vector<std::pair<int, int>> scan(const int& key1, const int& key2);
};

#endif //KV_STORE_RED_BLACK_TREE_H
