#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include "BPT_node.h"
#include "BPT_node.cpp"
#include "BufferManager.h"
#include "const.h"
#include "StringFunc.h"

extern BufferManager buffer_manager;

template <typename T>

class BPlusTree
{
private:
    typedef TreeNode<T>* Tree;// the head node of the tree
    std::string FileName;
    Tree root;
    Tree leafHead;
    int degree;
    int key_size;
    unsigned int level;
    unsigned int node_num;
    unsigned int key_num;
    struct tmp_Node
    {
        Tree pNode;
        bool isFind;
        unsigned int index;// the index of the node
    };
    
public:
    //node operations
    BPlusTree(std::string f_name, int key_size, int tree_degree);
    ~BPlusTree();
    void initTree();
    void drop_tree(Tree node);
    bool insert_key(T &key, int element);
    bool after_Insertion(Tree pNode);
    bool delete_key(T &key);
    bool after_Deletion(Tree pNode);
    void FindLeaf(Tree pNode, T key, tmp_Node &t_node);// find the leaf by key
    int search_element(T &key);//get element by key
    void search_range(T &key1, T &key2, std::vector<int> & element, int flag);//search and put result into element vector
    void print_leaf();
    //file operations
    void getFile(std::string fname);
    int getBlockNum(std::string table_name);
    void readFromDisk(char *p, char *end);
    void readFromDiskAll();
    void WBToDiskAll();
};











