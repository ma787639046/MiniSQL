#pragma once
#include <iostream>
#include <vector>
#include "BufferManager.h"

extern BufferManager buffer_manager;

template <typename T>
class TreeNode
{
public:
    unsigned int num; // number of keys
    std::vector <T> keys; // std::vector storing keys
    TreeNode * parent;// points to its parent
    std::vector <TreeNode*> child; //points to its child
    std::vector <int> element;
    TreeNode * next; // points to next leaf node
    bool isLeaf;// leaf node or not
    int degree;
public:
    TreeNode(int tree_degree, bool isleaf);// construction function
    ~TreeNode();
    bool isRoot();
    TreeNode * nextLeaf();
    unsigned int Add(T &key);//add the key
    unsigned int Add(T &key,int elem);//add the key
    bool Delete(unsigned int index);//delete node by index
    TreeNode * Split(T &key);// split the node
    bool Find(T key, unsigned int &index);// return index if key is found
    bool RangeFind(unsigned int index, T &key, std::vector<int>& elem_ret);//find nodes index<x<key
    bool RangeFind2(unsigned int index,std::vector<int>& elem_ret);//find nodes x > index
    //start from index, end to key
    void test();
};












