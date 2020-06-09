#include "BPT_tree.h"


class IndexManager
{
    //Index Type Selection
    std::map< std::string, BPlusTree<int>* > index_int_map;
    std::map< std::string, BPlusTree<float> > index_float_map;
    std::map< std::string, BPlusTree<string> > index_string_map;
public:
    IndexManager(std::string table_name);
    ~IndexManager();
    int get_degree(int type);
    int get_key_size(int type);
    void create_index(std::string f_path, int type);
    void drop_index(std::string f_path, )
};
