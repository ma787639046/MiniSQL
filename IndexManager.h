#pragma once
#include <iostream>
#include <map>
#include "CatalogManager.h"
#include "BPT_tree.h"
#include "BPT_tree.cpp"
#include "Table.h"


class IndexManager
{
    //Index Type Selection
    typedef std::map<std::string, BPlusTree<int> *> map_int;
    typedef std::map< std::string, BPlusTree<float> *> map_float;
    typedef std::map< std::string, BPlusTree<std::string> *> map_string;
    map_int index_int_map;
    map_float index_float_map;
    map_string index_string_map;
public:
    //########################### Need string sizes!!!! ###########################
    //constructor
    IndexManager(std::string table_name);
    
    //deconstructor
    ~IndexManager();
    
    //get the degree of BPT
    int get_degree(int type, int string_size);
    
    //generate index, BPT, and files.
    void generate_index(std::string file_name, int type, int string_size);
    
    // drop index, BPT, and files
    void drop_index(std::string file_name, int type);
    
    //insert key
    void insert_index(std::string file_name, key_ data, int block_id, int type);
    
    //delete key
    void delete_index(std::string file_name, key_ data, int type);
    
    //find key range, return their elements
    void find_range(std::string file_name, key_ data1, key_ data2, std::vector<int>& elem, int type1, int type2);
};



