#include "iostream"
#include "API.h"
#include "Relation.h"
#include "StringFunc.h"
#include "exceptions.h"

class Interpreter
{
    std::string query;
public:
    Interpreter();
    ~Interpreter();
    //decode query
    void decode_select();
    void decode_show();
    void decode_insert();
    void decode_delete();
    void decode_exit();
    //decode stuff
    void decode_file_read();
    void decode_table_create();
    void decode_table_drop();
    void decode_index_generate();
    void decode_index_delete();
    void decode();
    //get stuff
    void get_query();
    std::string fetch_word(int location, int & tail);
    std::string get_condition(int location, int & tail);
    int get_type(int location, int & tail);
    int get_len(int number);
    int get_len(float number);
    void norm();
};
