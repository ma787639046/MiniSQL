#include "BPT_tree.h"


template <class T>
BPlusTree<T>::BPlusTree(std::string f_name, int key_size, int tree_degree):FileName(f_name),key_num(0),level(0),node_num(0),root(NULL),leafHead(NULL),key_size(key_size),degree(tree_degree)
{
    initTree();
    readFromDiskAll();
}
template <class T>
BPlusTree<T>::~BPlusTree()
{
    drop_tree(root);
    key_num = 0;
    root = NULL;
    level = 0;
}
template <class T>
void BPlusTree<T>::initTree()
{
    root = new TreeNode<T>(degree, true);//is leaf
    key_num = 0;
    level = 1;
    node_num = 1;
    leafHead = root;
}
template <class T>
void BPlusTree<T>::drop_tree(Tree node)
{
    if(node!=NULL)
    {
        if(!node->isLeaf)//not leaf node
        {
            for(int i = 0; i <= node->num; i++)
            {
                drop_tree(node->child[i]);//pass down
                node->child[i] = NULL;
            }
        }
        delete node;
        node_num--;
        return;
    }
    else
    {
        return;
    }
}
template <class T>
bool BPlusTree<T>::insert_key(T &key, int element)
{
    tmp_Node t_node;
    if(root==NULL)
    {
        initTree();
    }
    FindLeaf(root, key, t_node);
    if(t_node.isFind)//is found, alread exsits
    {
        std::cout<<"the inserted key is already exsits.\n";
        return false;
    }
    t_node.pNode->Add(key,element);
    if(t_node.pNode->num == degree)//if the node is full after insertion
    {
        after_Insertion(t_node.pNode);
    }
    key_num++;
    return true;
}
template <class T>
bool BPlusTree<T>::after_Insertion(Tree pNode)
{
    T key;
    Tree newNode = pNode->Split(key);//split the full node, return the critical key, newNode is right-half
    node_num++;
    if(pNode->isRoot())//if the full node is root
    {
        Tree root = new TreeNode<T>(degree, false);//not a leaf
        if(root==NULL)
        {
            std::cout<<"Not enough space to create a new root node\n";
            return false;
        }
        level++;
        node_num++;
        this->root = root;
        pNode->parent = root;
        newNode->parent = root;
        root->Add(key);
        root->child[0] = pNode;
        root->child[1] = newNode;
        return true;
    }
    else// full node is not a root
    {
        Tree parent = pNode->parent;
        unsigned int index = parent->Add(key);//return the key's index
        parent->child[index+1] = newNode;
        newNode->parent = parent;
        
        if(parent->num == degree)//parent is full
        {
            return after_Insertion(parent);
        }
        return true;
    }
}
template <class T>
bool BPlusTree<T>::delete_key(T &key)
{
    tmp_Node t_node;
    if(root==NULL)
    {
        std::cout<<"Root is empty!\n";
        return false;
    }
    FindLeaf(root, key, t_node);
    if(!t_node.isFind)//not found
    {
        std::cout<<"Key not exsits!\n";
        return false;
    }
    if(t_node.pNode->isRoot())//deleting root
    {
        t_node.pNode->Delete(t_node.index);
        key_num--;
        return after_Deletion(t_node.pNode);
    }
    else// not root
    {
        if(t_node.index==0 && leafHead!=t_node.pNode)//index=0 and not leaf, the key exists in parents
        {
            unsigned int index;
            Tree new_parent = t_node.pNode->parent;
            bool isfind_inparent = new_parent->Find(key, index);
            while(!isfind_inparent)//not found, do upper
            {
                if(new_parent->parent!=NULL)
                {
                    new_parent = new_parent->parent;
                }
                else
                {
                    break;
                }
                isfind_inparent = new_parent->Find(key, index);
            }
            //replace the deleted position with latter
            new_parent->keys[index] = t_node.pNode->keys[1];
            //delete by index in node
            t_node.pNode->Delete(t_node.index);
            key_num--;
            return after_Deletion(t_node.pNode);
        }
        else//leaf node
        {
            t_node.pNode->Delete(t_node.index);
            key_num--;
            return after_Deletion(t_node.pNode);
        }
    }
}
template <class T>
bool BPlusTree<T>::after_Deletion(Tree pNode)
{
    unsigned int min_key_num = (degree-1)/2;
    //No need to adjust
    if(pNode->isLeaf && pNode->num >= min_key_num)//leaf and big enough
    {
        return true;
    }
    if(degree!=3 && !pNode->isLeaf && pNode->num>=min_key_num-1)// not leaf node, degree!=3, big enough
    {
        return true;
    }
    if(degree==3 && !pNode->isLeaf && pNode->num<0)// not leaf node, degree=3, empty
    {
        return true;
    }
    //need adjustment
    if(pNode->isRoot())//adjust root
    {
        if(pNode->num > 0)
        {
            return true;
        }
        
        if(root->isLeaf)// root is leaf
        {
            delete pNode;
            root = NULL;
            leafHead = NULL;
            level--;
            node_num--;
        }
        else//root is non-leaf
        {
            root = pNode->child[0];//make root's 1st child as root
            root->parent = NULL;
            delete pNode;
            level--;
            node_num--;
        }
    }
    else//adjust non-root
    {
        Tree parent = pNode->parent;
        Tree brother = NULL;
        if(pNode->isLeaf)//leaf node
        {
            unsigned int index = 0;
            parent->Find(pNode->keys[0], index);
            
            if(parent->child[0]!=pNode && index+1==parent->num)//pNode is rightest child
            {
                brother = parent->child[index];//pNode's left brother
                if(brother->num > min_key_num)//big enough, give one to pNode
                {
                    for(int i=pNode->num;i>0;i--)
                    {
                        pNode->keys[i] = pNode->keys[i-1];
                        pNode->element[i] = pNode->element[i-1];
                    }
                    pNode->keys[0] = brother->keys[brother->num-1];
                    pNode->element[0] = brother->element[brother->num-1];
                    brother->Delete(brother->num-1);
                    pNode->num++;
                    //update parent's key
                    parent->keys[index] = pNode->keys[0];
                    return true;
                }
                else//brother is small, shrink parent, combine brother and pNode
                {
                    parent->Delete(index);//delete parent
                    for(int i=0;i<pNode->num;i++)
                    {
                        brother->keys[brother->num+i] = pNode->keys[i];
                        brother->element[brother->num+i] = pNode->element[i];
                    }
                    brother->num += pNode->num;
                    brother->next = pNode->next;
                    delete pNode;
                    node_num--;
                    return after_Deletion(parent);//pass up to parent
                }
            }
            else//pNode is not rightest
            {
                //make brother to pNode's right
                if(parent->child[0] == pNode)//lefest child
                {
                    brother = parent->child[1];
                }
                else
                {
                    brother = parent->child[index+2];
                }
                if(brother->num > min_key_num)// big enough, give one to pNode
                {
                    pNode->keys[pNode->num] = brother->keys[0];
                    pNode->element[pNode->num] = brother->element[0];
                    pNode->num++;
                    brother->Delete(0);
                    //update parent's key
                    if(pNode == parent->child[0])
                    {
                        parent->keys[0] = brother->keys[0];
                    }
                    else
                    {
                        parent->keys[index+1] = brother->keys[0];
                    }
                    return true;
                }
                else// brother is small, merge
                {
                    for(int i=0;i<brother->num;i++)
                    {
                        pNode->keys[pNode->num+i] = brother->keys[i];
                        pNode->element[pNode->num+i] = brother->element[i];
                    }
                    if(pNode == parent->child[0])
                    {
                        parent->Delete(0);
                    }
                    else
                    {
                        parent->Delete(index+1);
                    }
                    pNode->num += brother->num;
                    pNode->next = brother->next;
                    delete brother;
                    node_num--;
                    return after_Deletion(parent);
                }
            }
        }
        else//not a leaf-node
        {
            unsigned int index = 0;
            parent->Find(pNode->child[0]->keys[0], index);
            if(parent->child[0]!=pNode && index+1 == parent->num)//pnode is rightest
            {
                brother = parent->child[index];//brother is left
                if (brother->num > min_key_num - 1)//bro big enough, give 1 to pNode
                {
                    pNode->child[pNode->num+1] = pNode->child[pNode->num];
                    for (unsigned int i = pNode->num; i > 0; i--)
                    {
                        pNode->child[i] = pNode->child[i-1];
                        pNode->keys[i] = pNode->keys[i-1];
                    }
                    pNode->child[0] = brother->child[brother->num];
                    pNode->keys[0] = parent->keys[index];
                    pNode->num++;
                    parent->keys[index] = brother->keys[brother->num-1];
                    if (brother->child[brother->num]!=NULL)
                    {
                        brother->child[brother->num]->parent = pNode;
                    }
                    brother->Delete(brother->num-1);
                    return true;
                }
                else// bro is small, merge
                {
                    brother->keys[brother->num] = parent->keys[index];
                    parent->Delete(index);
                    brother->num++;
                    for (int i = 0; i < pNode->num; i++)
                    {
                        brother->child[brother->num+i] = pNode->child[i];
                        brother->keys[brother->num+i] = pNode->keys[i];
                        brother->child[brother->num+i]->parent = brother;
                    }
                    brother->child[brother->num+pNode->num] = pNode->child[pNode->num];
                    brother->child[brother->num+pNode->num]->parent = brother;
                    brother->num += pNode->num;
                    delete pNode;
                    node_num --;
                    return after_Deletion(parent);
                }
            }
            else//pnode is not the most right
            {
                if (parent->child[0] == pNode)//most left
                {
                    brother = parent->child[1];
                }
                else
                {
                    brother = parent->child[index+2];
                }
                if (brother->num > min_key_num - 1)//bro big, give 1 to pNode
                {
                    pNode->child[pNode->num+1] = brother->child[0];
                    pNode->keys[pNode->num] = brother->keys[0];
                    pNode->child[pNode->num+1]->parent = pNode;
                    pNode->num++;
                    if (pNode == parent->child[0])
                    {
                        parent->keys[0] = brother->keys[0];
                    }
                    else
                    {
                        parent->keys[index+1] = brother->keys[0];
                    }
                    brother->child[0] = brother->child[1];
                    brother->Delete(0);
                    return true;
                }
                else//bro small, merge
                {
                    pNode->keys[pNode->num] = parent->keys[index];
                    if(pNode == parent->child[0])
                    {
                        parent->Delete(0);
                    }
                    else
                    {
                        parent->Delete(index+1);
                    }
                    pNode->num++;
                    for(int i = 0; i < brother->num; i++)
                    {
                        pNode->child[pNode->num+i] = brother->child[i];
                        pNode->keys[pNode->num+i] = brother->keys[i];
                        pNode->child[pNode->num+i]->parent = pNode;
                    }
                    pNode->child[pNode->num+brother->num] = brother->child[brother->num];
                    pNode->child[pNode->num+brother->num]->parent = pNode;
                    pNode->num += brother->num;
                    delete brother;
                    node_num--;
                    return after_Deletion(parent);
                }
            }
        }
    }
    return false;
}
//find the leaf node of the key
template <class T>
void BPlusTree<T>::FindLeaf(Tree pNode, T key, tmp_Node &t_node)
{
    unsigned int index = 0;
    if(pNode->Find(key, index))//find if key is in current node
    {
        if(pNode->isLeaf)// current node is leaf node
        {
            t_node.pNode = pNode;
            t_node.index = index;
            t_node.isFind = true;
        }
        else// not a leaf node
        {
            pNode = pNode->child[index+1];
            while(!pNode->isLeaf)//deepen until to leaf
            {
                pNode = pNode->child[0];
            }
            t_node.pNode = pNode;
            t_node.index = 0;
            t_node.isFind = true;
        }
    }
    else//not find
    {
        if(pNode->isLeaf)
        {
            t_node.pNode = pNode;
            t_node.index = index;
            t_node.isFind = false;
        }
        else
        {
            FindLeaf(pNode->child[index], key, t_node);
        }
    }
}
//get key's element
template <class T>
int BPlusTree<T>::search_element(T &key)
{
    if(root==NULL) return -1;
    tmp_Node t_node;
    FindLeaf(root, key, t_node);
    if(!t_node.isFind)//not found
    {
        return -1;
    }
    return t_node.pNode->element[t_node.index];
}
template <class T>
void BPlusTree<T>::search_range(T &key1, T &key2, std::vector<int> & element, int flag)
{
    if(root==NULL)
    {
        return;
    }
    
    if (flag == 2)//find > key1
    {
        tmp_Node t_node1;
        FindLeaf(root, key1, t_node1);
        bool finished = false;
        Tree pNode = t_node1.pNode;
        unsigned int index = t_node1.index;
        do
        {
            finished = pNode->RangeFind2(index, element);//find nodes x > t_node1.index, store them into element
            index = 0;
            if (pNode->next == NULL) break;
            else pNode = pNode->nextLeaf();
        } while (!finished);
    }
    else if (flag == 1)// find < key2
    {
        tmp_Node t_node2;
        FindLeaf(root, key2, t_node2);
        bool finished = false;
        Tree pNode = t_node2.pNode;
        unsigned int index = t_node2.index;
        do
        {
            finished = pNode->RangeFind2(index, element);
            index = 0;
            if (pNode->next == NULL) break;
            else pNode = pNode->nextLeaf();
        } while (!finished);
    }
    else// find   key1 < x < key2
    {
        tmp_Node t_node1, t_node2;
        FindLeaf(root, key1, t_node1);
        FindLeaf(root, key2, t_node2);
        bool finished = false;
        unsigned int index;
        if (key1 <= key2)
        {
            Tree pNode = t_node1.pNode;
            index = t_node1.index;//index = key1
            do
            {
                finished = pNode->RangeFind(index, key2, element);//find <key2
                index = 0;
                if (pNode->next == NULL) break;
                else pNode = pNode->nextLeaf();
            } while (!finished);
        }
        else//reverse
        {
            Tree pNode = t_node2.pNode;
            index = t_node2.index;
            do {
                finished = pNode->RangeFind(index, key1, element);
                index = 0;
                if (pNode->next == NULL)
                    break;
                else
                    pNode = pNode->nextLeaf();
            } while (!finished);
        }
    }
    //sort and make it unique
    std::sort(element.begin(),element.end());
    element.erase(unique(element.begin(), element.end()), element.end());//if not unique, erase the latter
    return;
}
template <class T>
void BPlusTree<T>::print_leaf()
{
    Tree p = leafHead;
    while(p != NULL)
    {
        p->test();
        p = p->nextLeaf();
    }
    return;
}


// File Operations:
template <class T>
void BPlusTree<T>::getFile(std::string fname)
{
    FILE* f = fopen(fname.c_str() , "r");
    if (f == NULL)
    {
        f = fopen(fname.c_str(), "w+");
        fclose(f);
        f = fopen(fname.c_str() , "r");
    }
    fclose(f);
    return;
}
template <class T>
int BPlusTree<T>::getBlockNum(std::string table_name)
{
    char* p;
    int block_num = -1;
    do
    {
        p = buffer_manager.getPage(table_name, block_num+1);
        block_num++;
    } while(p[0] != '\0');
    return block_num;
}
template <class T>
void BPlusTree<T>::readFromDisk(char *p, char *end)
{
    T key;//create a key
    int element;//create an element
    for(int i = 0; i < PAGESIZE; i++)
    {
        if (p[i] != '#')
        {
            return;
        }
        i += 2;
        char tmp[100];
        int j;
        for(j = 0; i < PAGESIZE && p[i] != ' '; i++)
        {
            tmp[j++] = p[i];
        }
        tmp[j] = '\0';
        std::string s(tmp);
        std::stringstream stream(s);
        //get the key
        stream >> key;
        memset(tmp, 0, sizeof(tmp));
        i++;
        for(j = 0; i < PAGESIZE && p[i] != ' '; i++)
        {
            tmp[j++] = p[i];
        }
        tmp[j] = '\0';
        std::string s1(tmp);
        std::stringstream stream1(s1);
        //get the element
        stream1 >> element;
        //put them into the tree
        insert_key(key, element);
    }
}
//read from disk all block_num times
template <class T>
void BPlusTree<T>::readFromDiskAll()
{
    std::string fname = INDEX_PATH + FileName;
    getFile(fname);
    int block_num = getBlockNum(fname);//table name is fname
    if(block_num <= 0)
    {
        block_num = 1;
    }
    //get page pointer from buffer manager
    for(int i=0;i<block_num;i++)
    {
        char *p = buffer_manager.getPage(fname, i);
        //read: insert the key and the element into the tree
        readFromDisk(p, p+PAGESIZE);
    }
}
template <class T>
void BPlusTree<T>::WBToDiskAll()
{
    std::string fname = INDEX_PATH + FileName;
    getFile(fname);
    int block_num = getBlockNum(fname);
    //get the leaf
    Tree ntmp = leafHead;
    int i, j;
    //go over each leaf
    for (j = 0, i = 0; ntmp != NULL; j++)
    {
        char *p = buffer_manager.getPage(fname, j);
        int offset = 0;
        memset(p, 0, PAGESIZE);
        //for each leaf, write p: p[offset] = ntmp->keys[i] + ntmp->element[i]
        for (i = 0; i < ntmp->num; i++)
        {
            p[offset++] = '#';
            p[offset++] = ' ';
            string_duplicate(p, offset, ntmp->keys[i]);
            p[offset++] = ' ';
            string_duplicate(p, offset, ntmp->element[i]);
            p[offset++] = ' ';
        }
        p[offset] = '\0';
        //make this page dirty
        int page_id = buffer_manager.getPageId(fname, j);
        buffer_manager.setDirty(page_id);
        ntmp = ntmp->next;
    }
    //if j still not full, set them empty, and make the rest blocks dirty
    while (j < block_num)
    {
        char *p = buffer_manager.getPage(fname, j);
        int page_id = buffer_manager.getPageId(fname, j);
        memset(p, 0, PAGESIZE);
        buffer_manager.setDirty(page_id);
        j++;
    }
    return;
}



//int main()
//{
//    // TreeNode a;
//    std::cout<<"hello, this is a BPlusTree\n";
//
//    return 0;
//}
