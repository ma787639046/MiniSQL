#include "Interpreter.h"
#include "API.h"
#include "Table.h"

Interpreter::Interpreter(){}
Interpreter::~Interpreter(){}

void Interpreter::decode_select()
{
    API API;
    CatalogManager cata_m;
    std::string table_name;
    std::vector<std::string> attribute_name;
    std::vector<std::string> cur_name;
    std::vector<condition> condition_select;
    std::string tmp_cur_name;
    std::string tmp_element;
    condition tmp_condition;
    std::string relation;
    Table ret_table;
    char op = 0;
    
    
}
