#include <iostream>
#include <vector>
#include "BPT_node.h"

using namespace std;
template <typename T>

class BPlusTree
{
private:
    typedef TreeNode<T>* Tree;// the head
    string FileName;
    Tree root;
    Tree leafHead;
    int degree;
    int KeySize;
    unsigned int level;
    unsigned int node_num;
    unsigned int key_num;
    struct tmp_Node
    {
        Tree pNode;
        bool isFind;
        unsigned int index;// the index of the key
    };
    
private: 
    void initTree();
    void getFile(string file_path);
    int getBlockNum(string table_name);
    void FindLeaf(Tree pNode, T key, tmp_Node &t_node);// find the leaf by key
    bool after_Insertion(Tree pNode);
    bool after_Deletion(Tree pNode);
};











