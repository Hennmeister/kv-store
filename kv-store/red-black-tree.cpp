#include <iostream>

enum Color { RED, BLACK };

class Node {
public:
    int data;
    bool color;
    Node* left, * right, * parent;

    Node(int data) {
        this->data = data;
        left = right = parent = nullptr;
        this->color = RED;
    }
};

class RedBlackTree {
private:
    Node* root;
    Node* nil;

    void rotateLeft(Node*&, Node*&);
    void rotateRight(Node*&, Node*&);
    void fixViolation(Node*&, Node*&);

public:
    RedBlackTree() {
        nil = new Node(-1);
        nil->color = BLACK;
        nil->left = nil->right = nil->parent = nil;
        root = nil;
    }

    void insert(const int& n);
};

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

void RedBlackTree::insert(const int& data) {
    Node* pt = root;
    Node* parent = nil;

    while (pt != nil) {
        parent = pt;

        if (data < pt->data)
            pt = pt->left;
        else
            pt = pt->right;
    }

    pt = new Node(data);
    pt->parent = parent;
    pt->left = nil;
    pt->right = nil;
    pt->color = RED;

    if (parent == nil)
        root = pt;
    else if (data < parent->data)
        parent->left = pt;
    else
        parent->right = pt;

    fixViolation(root, pt);
}
