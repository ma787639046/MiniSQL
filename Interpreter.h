#pragma once
#include <iostream>
#include "API.h"
#include "Relation.h"
#include "StringFunc.h"
#include "exceptions.h"
#include <time.h>

class Interpreter
{
    std::string query;
    std::string current_table_name;
public:
    //default constructor
    Interpreter();
    
    //constructor with table name input
    Interpreter(std::string cur_table_name);
    
    //deconstructor
    ~Interpreter();
    
    //decode query
    std::string decode_table_create();//create table
    void decode_select();//do SELECT *
    void decode_show();//print current table's info
    void decode_insert();//do INSERT
    void decode_delete();//do DELETE
    void decode_table_drop();//drop the table
    void decode_exit();//do EXIT
    std::string decode();//the unified function which contains the above parts
    
    //index management
    std::string decode_file_read();//read the file through table name
    void decode_index_generate();//generate the index
    void decode_index_delete();//delete the index
    
    //some small functions to implement basic needs
    void set_cur_table_name(std::string cur_table_name);//set current table name
    void read_in_command();//read command
    std::string fetch_word(int location, int & tail);//fetch next word in a query
    std::string get_condition(int location, int & tail);//get current condition
    int get_type(int location, int & tail);//get type
    int get_len(int number);//get word length
    int get_len(float number);//get word length
    void split_space(int is_file);//add space to split
    std::string catch_erro();//catch all erros
};
