#include "IndexManager.h"

IndexManager::IndexManager(std::string table_name)
{
    CatalogManager catalog;
    Attribute table_attr = catalog.getAttribute(table_name);
    for(int i=0;i<table_attr.num;i++)
    {
        if(table_attr.index[i]!=0)//it needs index
        {
            if(table_attr.type[i] > 0)//STRING
            {
                int string_size = table_attr.type[i];
                generate_index("INDEX_FILE_" + table_attr.name[i] + "_" + table_name, table_attr.type[i], string_size);
            }
            else
            {
                generate_index("INDEX_FILE_" + table_attr.name[i] + "_" + table_name, table_attr.type[i],-1);
            }
        }
    }
}

 IndexManager::~IndexManager()
{
    auto iter_int = index_int_map.begin();
    auto iter_float = index_float_map.begin();
    auto iter_string = index_string_map.begin();
    //use iterator to free the int data
    for(iter_int = index_int_map.begin();iter_int != index_int_map.end();iter_int++)
    {
        if(iter_int->second != NULL)
        {
            iter_int->second->WBToDiskAll();
            delete iter_int->second;
        }
    }
    //use iterator to free the float data
    for(iter_float = index_float_map.begin();iter_float != index_float_map.end();iter_float++)
    {
        if(iter_float->second != NULL)
        {
            iter_float->second->WBToDiskAll();
            delete iter_float->second;
        }
    }
    //use iterator to free the string data
    for(iter_string = index_string_map.begin();iter_string != index_string_map.end();iter_string++)
    {
        if(iter_string->second != NULL)
        {
            iter_string->second->WBToDiskAll();
            delete iter_string->second;
        }
    }
}
int IndexManager::get_degree(int type, int string_size)
{
    int degree;
    if(type==INT)
    {
        degree = (PAGESIZE-sizeof(int)) / (sizeof(int)+sizeof(int));
    }
    else if(type==FLOAT)
    {
        degree = (PAGESIZE-sizeof(int)) / (sizeof(float)+sizeof(int));
    }
    else if(type > 0)//string
    {
        degree = (PAGESIZE-sizeof(int)) / (string_size+sizeof(int));
    }
    else
    {
        std::cout<<"The data type is not supported!\n";
    }
    if(degree%2==0) degree--;
    return degree;
}
void IndexManager::generate_index(std::string file_name, int type, int string_size)//generate index, BPT, and files.
{
    int degree = get_degree(type, string_size);
    int size_of_key;
    if(type==INT)
    {
        size_of_key = sizeof(int);
        BPlusTree<int>* Tree = new BPlusTree<int>(file_name, size_of_key, degree);
        index_int_map.insert(std::map<std::string,BPlusTree<int>*>::value_type(file_name, Tree));
    }
    else if(type==FLOAT)
    {
        size_of_key = sizeof(float);
        BPlusTree<float>* Tree = new BPlusTree<float>(file_name, size_of_key, degree);
        index_float_map.insert(std::map<std::string,BPlusTree<float>*>::value_type(file_name, Tree));
    }
    else if(type > 0)//string
    {
        size_of_key = string_size;
        BPlusTree<std::string>* Tree = new BPlusTree<std::string>(file_name, size_of_key, degree);
        index_string_map.insert(std::map<std::string,BPlusTree<std::string>*>::value_type(file_name, Tree));
    }
}
void IndexManager::drop_index(std::string file_name, int type)
{
    if(type==INT)
    {
        auto iter_int = index_int_map.find(file_name);
        if(iter_int != index_int_map.end())
        {
            delete iter_int->second;// delete the data
            index_int_map.erase(iter_int);
        }
        else
        {
            std::cout<<"The tree of index being deleted is not found!\n";
        }
    }
    else if(type==FLOAT)
    {
        auto iter_float = index_float_map.find(file_name);
        if(iter_float != index_float_map.end())
        {
            delete iter_float->second;// delete the data
            index_float_map.erase(iter_float);
        }
        else
        {
            std::cout<<"The tree of index being deleted is not found!\n";
        }
    }
    else
    {
        auto iter_string = index_string_map.find(file_name);
        if(iter_string != index_string_map.end())
        {
            delete iter_string->second;// delete the data
            index_string_map.erase(iter_string);
        }
        else
        {
            std::cout<<"The tree of index being deleted is not found!\n";
        }
    }
}
//int IndexManager::find_index(std::string file_name, key_ data, int type)
//{
//    int ret = 0;
//    if(type==INT)
//    {
//        auto iter_int = index_int_map.find(file_name);
//        if(iter_int != index_int_map.end())
//        {
//            ret = iter_int->second->search_element(data.INT_VALUE);
//        }
//        else
//        {
//            std::cout<<"The tree of index is not found!\n";
//            ret = -1;
//        }
//    }
//    else if(type==FLOAT)
//    {
//        auto iter_float = index_float_map.find(file_name);
//        if(iter_float != index_float_map.end())
//        {
//            ret = iter_float->second->search_element(data.FLOAT_VALUE);
//        }
//        else
//        {
//            std::cout<<"The tree of index is not found!\n";
//            ret = -1;
//        }
//    }
//    else
//    {
//        auto iter_string = index_string_map.find(file_name);
//        if(iter_string != index_string_map.end())
//        {
//            ret = iter_string->second->search_element(data.STRING_VALUE);
//        }
//        else
//        {
//            std::cout<<"The tree of index is not found!\n";
//            ret = -1;
//        }
//    }
//    return ret;
//}

void IndexManager::insert_index(std::string file_name, key_ data, int block_id, int type)
{
    if(type==INT)//if type is int
    {
        auto iter_int = index_int_map.find(file_name);
        if(iter_int != index_int_map.end())
        {
            iter_int->second->insert_key(data.INT_VALUE,block_id);//get int BPlusTree's insert function
        }
        else
        {
            std::cout<<"The tree is not found!\n";
        }
    }
    else if(type==FLOAT)//if type is float
    {
        auto iter_float = index_float_map.find(file_name);
        if(iter_float != index_float_map.end())
        {
            iter_float->second->insert_key(data.FLOAT_VALUE,block_id);//get float BPlusTree's insert function
        }
        else
        {
            std::cout<<"The tree is not found!\n";
        }
    }
    else//if type is string
    {
        auto iter_string = index_string_map.find(file_name);
        if(iter_string != index_string_map.end())
        {
            iter_string->second->insert_key(data.STRING_VALUE,block_id);//get string BPlusTree's insert function
        }
        else
        {
            std::cout<<"The tree is not found!\n";
        }
    }
}
void IndexManager::delete_index(std::string file_name, key_ data, int type)//delete key
{
    if(type==INT)//if type is int
    {
        auto iter_int = index_int_map.find(file_name);
        if(iter_int != index_int_map.end())
        {
            iter_int->second->delete_key(data.INT_VALUE);//get int BPlusTree's delete function
        }
        else
        {
            std::cout<<"The tree is not found!\n";
        }
    }
    else if(type==FLOAT)//if type is float
    {
        auto iter_float = index_float_map.find(file_name);
        if(iter_float != index_float_map.end())
        {
            iter_float->second->delete_key(data.FLOAT_VALUE);//get float BPlusTree's delete function
        }
        else
        {
            std::cout<<"The tree is not found!\n";
        }
    }
    else//if type is string
    {
        auto iter_string = index_string_map.find(file_name);
        if(iter_string != index_string_map.end())
        {
            iter_string->second->delete_key(data.STRING_VALUE);//get string BPlusTree's delete function
        }
        else
        {
            std::cout<<"The tree is not found!\n";
        }
    }
}

void IndexManager::find_range(std::string file_name, key_ data1, key_ data2, std::vector<int>& elem, int type1, int type2)//find key range, return their elements
{
    if(type1 != type2)
    {
        std::cout<<"2 data-types are not comparable!\n";
        return;
    }
    //search_type = 1: find < key2
    //search_type = 2: find > key1
    //search_type = exception: key1 < x < key2
    int search_type;
    
    if(type1!=INT && type1!=FLOAT && type1<=0)//not int, not float, not string
    {
        search_type = 1;
    }
    else if(type2!=INT && type2!=FLOAT && type2<=0)//not int, not float, not string
    {
        search_type = 2;
    }
    else
    {
        search_type = 0;
    }
    
    if(type1==INT)
    {
        auto iter_int = index_int_map.find(file_name);
        if(iter_int != index_int_map.end())
        {
            iter_int->second->search_range(data1.INT_VALUE, data2.INT_VALUE, elem, search_type);
        }
        else
        {
            std::cout<<"The tree is not found!\n";
        }
    }
    else if(type1==FLOAT)
    {
        auto iter_float = index_float_map.find(file_name);
        if(iter_float != index_float_map.end())
        {
            iter_float->second->search_range(data1.FLOAT_VALUE, data2.FLOAT_VALUE, elem, search_type);
        }
        else
        {
            std::cout<<"The tree is not found!\n";
        }
    }
    else
    {
        auto iter_string = index_string_map.find(file_name);
        if(iter_string != index_string_map.end())
        {
            iter_string->second->search_range(data1.STRING_VALUE, data2.STRING_VALUE, elem, search_type);
        }
        else
        {
            std::cout<<"The tree is not found!\n";
        }
    }
}

//int main()
//{
////    IndexManager index_manager;
//    std::cout<<"This is index manager.cpp\n";
//}

