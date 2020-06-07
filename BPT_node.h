#include <iostream>
#include <vector>
using namespace std;
template <typename T>

class TreeNode
{
public:
    unsigned int num; // number of keys
    vector <T> keys; // vector storing keys
    TreeNode * parent;// points to its parent
    vector <TreeNode*> child; //points to its child
    vector <int> element;
    TreeNode * next; // points to next leaf node
    bool isLeaf;// leaf node or not
    int degree;
public:
    TreeNode(int tree_degree, bool isleaf);// construction function
    ~TreeNode();
    bool isRoot();
    bool Find(T key, unsigned int &index);// return index if key is found
    unsigned int Add(T &key);//add the key
    unsigned int Add(T &key,int elem);//add the key
    bool Delete(unsigned int index);//delete node by index
    TreeNode * Split(T &key);// split the node
    TreeNode * nextLeaf();
    bool RangeFind(unsigned int index, T &key, vector<int>& elem_ret);//find nodes index<x<key
    bool RangeFind2(unsigned int index,vector<int>& elem_ret);//find nodes x > index
    //start from index, end to key
    void test();
};












