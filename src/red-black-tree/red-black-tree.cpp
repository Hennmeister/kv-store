#include <iostream>
#include <vector>
#include "red-black-tree.h"

using namespace std;

Node::Node(int key, int value) {
    this->key = key;
    this->value = value;
    left = right = parent = nullptr;
    this->color = RED;
}

RedBlackTree::RedBlackTree(const int& key, const int& value) {
    nil = new Node(0, 0);
    nil->color = BLACK;
    nil->left = nil->right = nil->parent = nil;
    root = nil;
}

void RedBlackTree::rotateLeft(Node*& root, Node*& pt) {
    Node* pt_right = pt->right;

    pt->right = pt_right->left;

    if (pt->right != nil)
        pt->right->parent = pt;

    pt_right->parent = pt->parent;

    if (pt->parent == nil)
        root = pt_right;

    else if (pt == pt->parent->left)
        pt->parent->left = pt_right;

    else
        pt->parent->right = pt_right;

    pt_right->left = pt;
    pt->parent = pt_right;
}

void RedBlackTree::rotateRight(Node*& root, Node*& pt) {
    Node* pt_left = pt->left;

    pt->left = pt_left->right;

    if (pt->left != nil)
        pt->left->parent = pt;

    pt_left->parent = pt->parent;

    if (pt->parent == nil)
        root = pt_left;

    else if (pt == pt->parent->left)
        pt->parent->left = pt_left;

    else
        pt->parent->right = pt_left;

    pt_left->right = pt;
    pt->parent = pt_left;
}

void RedBlackTree::fixViolation(Node*& root, Node*& pt) {
    Node* parent_pt = nil;
    Node* grand_parent_pt = nil;

    while ((pt != root) && (pt->color != BLACK) &&
        (pt->parent->color == RED)) {

        parent_pt = pt->parent;
        grand_parent_pt = pt->parent->parent;

        if (parent_pt == grand_parent_pt->left) {

            Node* uncle_pt = grand_parent_pt->right;

            if (uncle_pt->color == RED) {
                grand_parent_pt->color = RED;
                parent_pt->color = BLACK;
                uncle_pt->color = BLACK;
                pt = grand_parent_pt;
            }
            else {
                if (pt == parent_pt->right) {
                    rotateLeft(root, parent_pt);
                    pt = parent_pt;
                    parent_pt = pt->parent;
                }
                rotateRight(root, grand_parent_pt);
                std::swap(parent_pt->color, grand_parent_pt->color);
                pt = parent_pt;
            }
        }
        else {
            Node* uncle_pt = grand_parent_pt->left;

            if (uncle_pt->color == RED) {
                grand_parent_pt->color = RED;
                parent_pt->color = BLACK;
                uncle_pt->color = BLACK;
                pt = grand_parent_pt;
            }
            else {
                if (pt == parent_pt->left) {
                    rotateRight(root, parent_pt);
                    pt = parent_pt;
                    parent_pt = pt->parent;
                }

                rotateLeft(root, grand_parent_pt);
                std::swap(parent_pt->color, grand_parent_pt->color);
                pt = parent_pt;
            }
        }
    }

    root->color = BLACK;
}

void RedBlackTree::put(const int& key, const int& value) {
    if (key < min_key) {
        min_key = key;
    }
    if (key > max_key) {
        max_key = key;
    }

    Node* pt = root;
    Node* parent = nil;

    while (pt != nil) {
        parent = pt;

        if (key < pt->key)
            pt = pt->left;
        else
            pt = pt->right;
    }


    pt = new Node(key, value);
    pt->parent = parent;
    pt->left = nil;
    pt->right = nil;
    pt->color = RED;

    if (parent == nil) {
        root = pt;
        root->color = BLACK;
    }
    else if (key < parent->key)
        parent->left = pt;
    else
        parent->right = pt;

    fixViolation(root, pt);
}

int RedBlackTree::get(const int &key) {
    Node *curr = root;
    while (curr != nil) {
        if (curr->key == key) {
            return curr->value;
        }
        else if (curr->key < key) {
            curr = curr->right;
        }
        else {
            curr = curr->left;
        }
    }

    return NULL; // TODO: Determine what to return on key miss
}

vector<std::pair<int, int>> RedBlackTree::scan(const int &key1, const int &key2) {
    std::vector<std::pair<int, int>> kv_pairs = {};
    std::vector<Node*> stack = {};

    if (root != nil) {
        stack.push_back(root);
    }


    while (!stack.empty()) {
        Node *curr = stack.back();
        stack.pop_back();

        if (curr == nil) {
            continue;
        }
        if (curr->key >= key1) {
            stack.push_back(curr->left);
        }
        if (curr->key >= key1 && curr->key <= key2){
            kv_pairs.push_back(std::pair(curr->key, curr->value));
        }
        if (curr->key <= key2) {
            stack.push_back(curr->right);
        }
    }

    return kv_pairs;
}

