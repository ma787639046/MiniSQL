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
    int key_size;
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
    void getFile(string fname);
    int getBlockNum(string table_name);
    void FindLeaf(Tree pNode, T key, tmp_Node &t_node);// find the leaf by key
    bool after_Insertion(Tree pNode);
    bool after_Deletion(Tree pNode);
    
public:
    BPlusTree(string f_name, int key_size, int tree_degree);
    ~BPlusTree();
    int search_element(T &key);//get element by key
    bool insert_key(T &key, int element);
    bool delete_key(T &key);
    void drop_tree(Tree node);
    void searchRange(T &key1, T &key2, vector<int> & element, int flag);//search and put result into element vector
    void print_leaf();
    
    //file operations
    void readFromDiskAll();
    void WBToDiskAll();
    void readFromDisk(char *p, char *end);
};











