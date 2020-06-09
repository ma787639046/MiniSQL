#include "BPT_tree.h"
#include "Table.h"

class IndexManager
{
    //Index Type Selection
//    typedef std::map<std::string, BPlusTree<int> *> int_map;
//    typedef std::map< std::string, BPlusTree<float> > float_map;
//    typedef std::map< std::string, BPlusTree<std::string> > string_map;
    std::map<std::string, BPlusTree<int> *> index_int_map;
    std::map< std::string, BPlusTree<float> > index_float_map;
    std::map< std::string, BPlusTree<std::string> > index_string_map;
public:
    //########################### Need string sizes!!!! ###########################
    IndexManager(std::string table_name, int * string_sizes);
    ~IndexManager();
    int get_degree(int type, int string_size);//get the degree of BPT
    void generate_index(std::string file_name, int type, int string_size);//generate index, BPT, and files.
    void drop_index(std::string file_name, int type);// drop index, BPT, and files
    int find_index(std::string file_name, key_ data, int type)//find key
    void insert_index(std::string file_name, key_ data, int block_id, int type);//insert key
    void delete_index(std::string file_name, key_ data, int type);//delete key
    void find_range(std::string file_name, key_ data1, key_ data2, std::vector<int>& elem, int type1, int type2);//find key range, return their elements
};



