#include "BPT_tree.h"
#include "Table.h"

//define the key type and store the key
class KEY_:key_
{
public:
    int type_selection;//int=0, float=1, string=2
    //in class key_:
//    int INT_VALUE;
//    float FLOAT_VALUE;
//    std::string STRING_VALUE;
};

class IndexManager
{
    //Index Type Selection
    int int_type = 0;
    int float_type = 1;
    int string_type = 2;
    std::map< std::string, BPlusTree<int>* > index_int_map;
    std::map< std::string, BPlusTree<float> > index_float_map;
    std::map< std::string, BPlusTree<std::string> > index_string_map;
public:
    IndexManager(std::string table_name);
    ~IndexManager();
    int get_degree(int type);//get the degree of BPT
    int get_key_size(int type);//get the size of different keys
    void generate_index(std::string f_path, int type);//generate index, BPT, and files.
    void drop_index(std::string f_path, int type);// drop index, BPT, and files
    int find_index(std::string f_path, KEY_ data);//find key
    void insert_index(std::string file_path, KEY_ data, int block_id);//insert key
    void delete_index(std::string file_path, KEY_ data);//delete key
    void find_range(std::string file_path, KEY_ data1, KEY_ data2, std::vector<int>& elem);//find key range, return their elements
};
