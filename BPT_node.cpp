#include <iostream>
#include <vector>
#include "BPT_node.h"

// construction function
template <class T>
TreeNode<T>::TreeNode(int tree_degree, bool isleaf): num(0),parent(NULL),next(NULL),isLeaf(isleaf),degree(tree_degree)
{
    for(unsigned int i=0;i<degree+1;i++)
    {
        child.push_back(NULL);
        keys.push_back(T());
        element.push_back(int());
    }
    child.push_back(NULL);
}

template <class T>
TreeNode<T>::~TreeNode(){}

template <class T>
bool TreeNode<T>::isRoot()
{
    if(parent==NULL) return true;
    else return false;
}

template <class T>
TreeNode<T>* TreeNode<T>::nextLeaf()
{
    return next;
}

//add key for non-leaf node and return its index
template <class T>
unsigned int TreeNode<T>::Add(T &key)
{
    if(num==0)
    {
        num++;
        keys[0] = key;
        return 0;
    }
    unsigned int index = 0;
    //find the position for insertion
    bool exist = Find(key, index);
    if(!exist)
    {
        //adjust the keys and child nodes
        for(int i=num; i>index; i--)
        {
            keys[i] = key[i-1];
        }
        keys[index] = key;
        for(int i=num+1;i>index+1;i--)
        {
            child[i] = child[i-1];
        }
        child[index+1] = NULL;
        num++;
        return index;
    }
    else
    {
        std::cout<<"Inserted Key already exists.\n";
        return 0;
    }
}

//add the key for leaf node and return the index
template <class T>
unsigned int TreeNode<T>::Add(T &key, int elem)
{
    if(!isLeaf) return 0;
    if(num==0)//empty
    {
        num++;
        keys[0] = key;
        element[0] = elem;
        return 0;
    }
    unsigned int index = 0;
    bool isexist = Find(key,index);
    if(!isexist)
    {
        for(int i=num;i>index;i--)
        {
            keys[i] = keys[i-1];
            element[i] = element[i-1];
        }
        num++;
        keys[index] = key;
        element[index] = elem;
        return index;
    }
    else
    {
        std::cout<<"Added Leaf node already exsits.\n";
        return 0;
    }
}

template <class T>
bool TreeNode<T>::Delete(unsigned int index)
{
    if(index > num) return false;
    if(!isLeaf)
    {
        for(int i=index;i<num-1;i++)
        {
            keys[i] = keys[i+1];
            child[i+1] = child[i+2];
        }
        keys[num-1] = T();
        child[num] = NULL;
        num--;
        return true;
    }
    else
    {
        for(int i=index;i<num-1;i++)
        {
            keys[i] = keys[i+1];
            element[i] = element[i+1];
        }
        keys[num-1] = T();
        element[num-1] = int();
        num--;
        return true;
    }
}

template <class T>
TreeNode<T> * TreeNode<T>::Split(T &key)
{
    int min_Node_num = (degree-1)/2;
    //create a new node to return
    TreeNode* newNode = new TreeNode(degree, this->isLeaf);
    if(!isLeaf)//not a leaf node
    {
        key = keys[min_Node_num];
        //make new node with right-half
        for(int i = min_Node_num+1; i <= degree;i++)
        {
            int new_index = i - min_Node_num - 1;
            newNode->child[new_index] = this->child[i];
            newNode->child[new_index]->parent = newNode;
            this->child[i] = NULL;//make right-half empty
        }
        for(int i=min_Node_num+1;i <= degree-1;i++)
        {
            int new_index = i - min_Node_num - 1;
            newNode->keys[new_index] = this->keys[i];//get keys
            this->keys[i] = T();//make right-half empty
        }
        this->keys[min_Node_num] = T();
        newNode->parent = this->parent;
        newNode->num = min_Node_num;
        this->num = min_Node_num;
    }
    return newNode;
}
//find index by key
template <class T>
bool TreeNode<T>::Find(T key, unsigned int &index)
{
    if(num==0)
    {
        index = 0;
        return false;
    }
    if(key < key[0])//min
    {
        index = 0;
        return false;
    }
    if(key > keys[num-1])//max
    {
        index = num;
        return false;
    }
    int linear_value = 50;
    if(num < linear_value)//do linear search
    {
        for(int i=0;i<num;i++)
        {
            if(key==keys[i])
            {
                index = i;
                return true;
            }
            if(keys[i] < key)
            {
                continue;
            }
            else
            {
                index = i;
                return false;
            }
        }
    }
    else// do 1/2 search
    {
        int left = 0;
        int right = num - 1;
        int find = 0;
        while(left < right + 1)
        {
            find = (right + left) / 2;
            if (keys[find] == key)
            {
                index = find;
                return true;
            }
            else if (keys[find] < key)
            {
                left = find;
            }
            else if (keys[find] > key)
            {
                right = find;
            }
        }
        if (keys[left] >= key)
        {
            index = left;
            if(keys[left] == key) return true;
            else return false;
        }
        else if (keys[right] >= key)
        {
            index = right;
            if(keys[right] == key) return true;
            else return false;
        }
        else if(keys[right] < key)
        {
            index = right++;
            return false;
        }
    }
    return false;
}

//range find elements between index and key, return the elem_ret std::vector
template<class T>
bool TreeNode<T>::RangeFind(unsigned int index, T &key, std::vector<int>& elem_ret)
{
    int i = 0;
    for(i=index;keys[i]<=key&&i<num;i++)
    {
        elem_ret.push_back(element[i]);
    }
    if(keys[i] > key) return false;
    else return true;
}

//find nodes x > index
template <class T>
bool TreeNode<T>::RangeFind2(unsigned int index,std::vector<int>& elem_ret)
{
    for(int i=index;i<num;i++)
    {
        elem_ret.push_back(element[i]);
    }
    return false;
}

//std::cout to test
template <class T>
void TreeNode<T>::test()
{
    for(int i=0;i<num;i++)
    {
        std::std::cout<<" -> "<<keys[i];
    }
    std::cout<<"\n";
}
