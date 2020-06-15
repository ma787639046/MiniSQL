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
    void decode_select();//do SELECT *
    void decode_show();//print current table's info
    void decode_insert();//do INSERT
    void decode_delete();//do DELETE
    void decode_exit();//do EXIT
    //decode stuff
    void decode_file_read();
    void decode_table_create();
    void decode_table_drop();
    void decode_index_generate();
    void decode_index_delete();
    void decode();
    //get stuff
    void read_in_command();//read command
    std::string fetch_word(int location, int & tail);//fetch next word in a query
    std::string get_condition(int location, int & tail);//get current condition
    int get_type(int location, int & tail);//get type
    int get_len(int number);//get word length
    int get_len(float number);//get word length
    void split_space();//add space to split
    void catch_erro();//catch all erros
};
