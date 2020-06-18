#pragma once
#include <iostream>
#include "API.h"
#include "Relation.h"
#include "StringFunc.h"
#include "exceptions.h"

class Interpreter
{
    std::string query;
    std::string current_table_name;
public:
    Interpreter();
    Interpreter(std::string cur_table_name);
    ~Interpreter();
    //decode query
    void decode_select();//do SELECT *
    void decode_show();//print current table's info
    void decode_insert();//do INSERT
    void decode_delete();//do DELETE
    void decode_exit();//do EXIT
    //decode stuff
    void decode_file_read();
    std::string decode_table_create();
    void decode_table_drop();
    void decode_index_generate();
    void decode_index_delete();
    std::string decode();
    //get stuff
    void set_cur_table_name(std::string cur_table_name);
    void read_in_command();//read command
    std::string fetch_word(int location, int & tail);//fetch next word in a query
    std::string get_condition(int location, int & tail);//get current condition
    int get_type(int location, int & tail);//get type
    int get_len(int number);//get word length
    int get_len(float number);//get word length
    void split_space();//add space to split
    std::string catch_erro();//catch all erros
};
