#include "Interpreter.h"

Interpreter::Interpreter()
{
    current_table_name = "";
    query = "";
}

Interpreter::Interpreter(std::string cur_table_name)
{
    current_table_name = cur_table_name;
    query = "";
}

void Interpreter::set_cur_table_name(std::string cur_table_name)
{
    current_table_name = cur_table_name;
}

Interpreter::~Interpreter(){}

void Interpreter::decode_select()
{
    int cur_index;
    
    std::string table_name;
    std::string tmp_cur_attr_name;
    std::string tmp_element;
    std::string condition;
    std::vector<std::string> attribute_name;
    
    API API;
    CatalogManager catalog_manager;
    
    Relation tmp_relation;
    std::vector<Relation> relation_select;
    
    Table ret_table;
    
    //select * from ...
    if(fetch_word(7, cur_index) == "*")//select all
    {
        cur_index++;
    }
    else//not select all
    {
        std::cout<<"No * detected in query\n";
        throw input_format_error();
    }
    
    //no "from" in select query
    if(query.substr(cur_index,4) != "from")
    {
        std::cout<<"No from in select query\n";
        throw input_format_error();
    }
    //get "from"
    cur_index += 5;
    //get table name after "from"
    table_name = fetch_word(cur_index, cur_index);
    if(!catalog_manager.havetable(table_name))
    {
        std::cout<<"No such table named "<<table_name<<" \n";
        throw table_not_exist();
    }
    //get attr
    Attribute tmp_attr = catalog_manager.getAttribute(table_name);

    //select all, push them into attribute_name
    for(int i=0;i<tmp_attr.num;i++)
    {
        attribute_name.push_back(tmp_attr.name[i]);
    }
    
    cur_index++;
    if(query[cur_index]=='\0')//no "where" condition
    {
        ret_table = API.selectRecord(table_name, relation_select);
    }
    else
    {
        //check "where" in query
        if(query.substr(cur_index,5) != "where")
        {
            std::cout<<"No where in query\n";
            throw input_format_error();
        }
        cur_index += 6;
        
        //get conditions until break
        while(1)
        {
            tmp_cur_attr_name = fetch_word(cur_index, cur_index);
            int trash;
            // not have this attribute
            if(!catalog_manager.haveAttribute(table_name, tmp_cur_attr_name, trash))
            {
                std::cout<<"Not have this attribute after where\n";
                throw attribute_not_exist();
            }
            //get the attribute name
            tmp_relation.attributeName = tmp_cur_attr_name;
            //get the condition <= x
            condition = get_condition(cur_index+1, cur_index);
            if(condition == "<") tmp_relation.sign = LESS;
            else if(condition == "< ="||condition == "<=") tmp_relation.sign = LESS_OR_EQUAL;
            else if(condition == "=") tmp_relation.sign = EQUAL;
            else if(condition == ">") tmp_relation.sign = GREATER;
            else if(condition == "> ="||condition == ">=") tmp_relation.sign = GREATER_OR_EQUAL;
            else if(condition == "< >"||condition == "<>") tmp_relation.sign = NOT_EQUAL;
            else
            {
                std::cout<<"No such condition\n";
                throw input_format_error();
            }
            
            tmp_element = fetch_word(cur_index+1, cur_index);//get the number
            for(int i=0;i<tmp_attr.num;i++)
            {
                if(tmp_cur_attr_name == tmp_attr.name[i])//find the attribute to be selected under relation
                {
                    tmp_relation.key.type = (keyType)tmp_attr.type[i];
                    if(tmp_relation.key.type==INT)//-1
                    {
                        try
                        {
                            tmp_relation.key.INT_VALUE = string2num<int>(tmp_element);
                        }
                        catch(...)
                        {
                            std::cout<<"Type convert failed\n";
                            throw key_type_conflict();
                        }
                    }
                    else if(tmp_relation.key.type==FLOAT)//0
                    {
                        try
                        {
                            tmp_relation.key.FLOAT_VALUE = string2num<float>(tmp_element);
                        }
                        catch(...)
                        {
                            std::cout<<"Type convert failed\n";
                            throw key_type_conflict();
                        }
                    }
                    else//string
                    {
                        try
                        {
                            //if string == "xxx' or 'xxx"
                            if(tmp_element[0]=='\'' || tmp_element[tmp_element.length()-1]=='\'')
                            {
                                if(tmp_element[0]=='"' || tmp_element[tmp_element.length()-1]=='"' )
                                {
                                    std::cout<<"Wriong format of string\n";
                                    throw input_format_error();
                                }
                            }
                            tmp_relation.key.STRING_VALUE = tmp_element.substr(1, tmp_element.length() - 2);
                        }
                        catch(input_format_error)
                        {
                            std::cout<<"Input format error\n";
                            throw input_format_error();
                        }
                        catch(...)
                        {
                            throw key_type_conflict();
                        }
                    }
                    break;
                }
            }
            //push this whole condition in
            relation_select.push_back(tmp_relation);
            if(query[cur_index+1] == '\0')//end of conditions
            {
                break;
            }
            else if(query.substr(cur_index+1,3) != "and")//relations are not ands
            {
                std::cout<<"Input format error: no condition \"and\" found\n";
                throw 1;
            }
            fetch_word(cur_index+1, cur_index);//eat the "and"
            cur_index++;//read in next condition
        }
        ret_table = API.selectRecord(table_name, relation_select);
    }//done with query decode, records & relationships fetching
    
    Attribute current_attr = ret_table.attribute;
    int attr_pointer[32] = {0};
    if(attribute_name.size()!=0)//exsits attribute selected
    {
        for(int i=0;i<attribute_name.size();i++)//for each attribute
        {
            for(int j=0;j<current_attr.num;j++)//find them in ret_table
            {
                if(current_attr.name[j] == attribute_name[i])
                {
                    attr_pointer[i] = j;
                }
            }
        }
    }
    else
    {
        for(int j=0;j<current_attr.num;j++)
        {
            attr_pointer[j] = j;
        }
    }
    std::vector<Tuple> ret_tuple = ret_table.getTuple();
    
    //get the max attr_key length from attribute name
    int length_max = -5;
    for(int i=0;i<attribute_name.size();i++)
    {
        if((int)current_attr.name[attr_pointer[i]].length() > length_max)
        {
            length_max = (int)current_attr.name[attr_pointer[i]].length();
        }
    }
    for(int index=0;index<attribute_name.size();index++)//get the max length from attribute value
    {
        int type = current_attr.type[attr_pointer[index]];
        if(type == -1)//int
        {
            for(int i=0;i<ret_tuple.size();i++)
            {
                if(length_max < get_len(ret_tuple[i].getKeys()[attr_pointer[index]].INT_VALUE))
                {
                    length_max = get_len(ret_tuple[i].getKeys()[attr_pointer[index]].INT_VALUE);
                }
            }
        }
        else if(type == 0)//float
        {
            for(int i=0;i<ret_tuple.size();i++)
            {
                if(length_max < get_len(ret_tuple[i].getKeys()[attr_pointer[index]].FLOAT_VALUE))
                {
                    length_max = get_len(ret_tuple[i].getKeys()[attr_pointer[index]].FLOAT_VALUE);
                }
            }
        }
        else//string
        {
            for(int i=0;i<ret_tuple.size();i++)
            {
                if(length_max < ret_tuple[i].getKeys()[attr_pointer[index]].STRING_VALUE.length())
                {
                    length_max = (int)ret_tuple[i].getKeys()[attr_pointer[index]].STRING_VALUE.length();
                }
            }
        }
    }
    length_max++;
    
    //for each attribute, print out the result
    for(int index=0;index<attribute_name.size();index++)
    {
        //for i < (max_lenth-current_length)/2, left part
        for(int i=0;i<(length_max-current_attr.name[attr_pointer[index]].length())/2;i++)
        {
            std::cout<<" ";
        }
        std::cout<<current_attr.name[attr_pointer[index]].c_str();
        //for i < (max_lenth-current_length)/2, right part
        for(int i=0; i < (length_max - current_attr.name[attr_pointer[index]].length()) / 2;i++)
        {
            std::cout<<" ";
        }
        
        //not the last attribute
        if(index!=attribute_name.size()-1)
        {
            std::cout<<"|";
        }
        else
        {
            std::cout<<"\n";
        }
    }
    //create table bottom
    for(int i=0;i<attribute_name.size()*(length_max+1);i++) std::cout<<"-";
    std::cout<<"\n";
    
    //for each tuple
    for(int index=0;index<ret_tuple.size();index++)
    {
        for(int i=0;i<attribute_name.size();i++)
        {
            if(ret_tuple[index].getKeys()[attr_pointer[i]].type == -1)//int
            {
                int lenth = ret_tuple[index].getKeys()[attr_pointer[i]].INT_VALUE;
                lenth = get_len(lenth);
                //left part
                for(int j=0;j<(length_max-lenth)/2;j++) std::cout<<" ";
                std::cout<<ret_tuple[index].getKeys()[attr_pointer[i]].INT_VALUE;
                //right part
                for(int j=0;j<(length_max-lenth)/2;j++) std::cout<<" ";
                
                //not the last attribute
                if(i != attribute_name.size()-1) std::cout<<"|";
                //last attribute
                else std::cout<<"\n";
                
            }
            else if(ret_tuple[index].getKeys()[attr_pointer[i]].type == 0)//float
            {
                int lenth = get_len(ret_tuple[index].getKeys()[attr_pointer[i]].FLOAT_VALUE);
                //left part
                for(int j=0;j<(length_max-lenth)/2;j++) std::cout<<" ";
                printf("%.3f",ret_tuple[index].getKeys()[attr_pointer[i]].FLOAT_VALUE);
                //right part
                for(int j=0;j<(length_max-lenth)/2;j++) std::cout<<" ";
                
                //not the last attribute
                if(i != attribute_name.size()-1) std::cout<<"|";
                //last attribute
                else std::cout<<"\n";
                
            }
            else//string
            {
                std::string s_tmp = ret_tuple[index].getKeys()[attr_pointer[i]].STRING_VALUE;
                //not the last attribute
                if(i != attribute_name.size()-1)
                {
                    for(int j=0;j<(length_max-s_tmp.length())/2;j++) std::cout<<" ";
                    std::cout<<s_tmp.c_str();
                    for(int j=0;j<(length_max-s_tmp.length())/2;j++) std::cout<<" ";
                    std::cout<<"|";
                }
                else
                {
                    s_tmp = ret_tuple[index].getKeys()[i].STRING_VALUE;//???
                    for(int j=0;j<(length_max-s_tmp.length())/2;j++) std::cout<<" ";
                    std::cout<<s_tmp.c_str();
                    for(int j=0;j<(length_max-s_tmp.length())/2;j++) std::cout<<" ";
                    std::cout<<"\n";
                }
            }
        }
    }    
}
//print current table's info
void Interpreter::decode_show()
{
CatalogManager catalog_manager;
std::string table_name;
int cur_p;
//eat the first word and return cur_p
fetch_word(0, cur_p);
//get the table name
table_name = fetch_word(cur_p + 1, cur_p);
if (query[cur_p + 1] != '\0')
{
    std::cout << "There is more word after table name\n";
    throw 1;
}
//print the info of current table
catalog_manager.showCatalog(table_name);
}


void Interpreter::decode_insert()
{
    
    API api;
    CatalogManager catalog_manager;
    Attribute attr_find;
    Tuple cur_tuple;
    int cur_p;
    //get being inserted table name
    std::string table_name = fetch_word(12, cur_p);

    if (!catalog_manager.havetable(table_name))
    {
        std::cout << "table named "<<table_name<<" not exsits\n";
        throw table_not_exist();
    }
    if (query.substr(7, 4) != "into")//no into after INSERT
    {
        std::cout << "no into after insert\n";
        throw input_format_error();
    }

    


    if (query.substr(cur_p + 1, 6) != "values")//no values after table name
    {
        std::cout << "no values after table name";
        throw input_format_error();
    }

    cur_p += 8;//eat VALUES

    if (query[cur_p] != '(')
    {
        std::cout << "no ( after values\n";
        throw input_format_error();
    }

    attr_find = catalog_manager.getAttribute(table_name);
    cur_p--;
    int n_insert = 0;//make it like top in a stack
    //check every attr's content until the end
    while (query[cur_p + 1] != '\0' && query[cur_p + 1] != ')')
    {
        if (n_insert > attr_find.num)
        {
            std::cout << "Less attribute than expected\n";
            throw 1;
        }

        cur_p += 3;//eat (
        std::string value_insert = fetch_word(cur_p, cur_p);
        key_ key_insert;
        key_insert.type = attr_find.type[n_insert];

        //start inserting
        if (key_insert.type == -1)//int
        {
            try
            {
                key_insert.INT_VALUE = string2num<int>(value_insert);
            }
            catch (...)
            {
                std::cout << "int value insert failed\n";
                throw key_type_conflict();
            }
        }
        else if (key_insert.type == 0)//float
        {
            try
            {
                key_insert.FLOAT_VALUE = string2num<float>(value_insert);
            }
            catch (...)
            {
                std::cout << "float value insert failed\n";
                throw key_type_conflict();
            }
        }
        else//string
        {
            try
            {
                //if there is no ' or "
                if (value_insert[0] != '\'' || value_insert[value_insert.length() - 1] != '\'')
                {
                    if (value_insert[0] != '"' || value_insert[value_insert.length() - 1] != '"')
                    {
                        std::cout << "no \' or \" detected\n";
                        throw input_format_error();
                    }

                }
                //length not right
                if (value_insert.length() - 1 > attr_find.type[n_insert])
                {
                    std::cout << "the string length is wrong\n";
                    throw input_format_error();
                }

                //insert
                key_insert.STRING_VALUE = value_insert.substr(1, value_insert.length() - 2);
            }
            catch (input_format_error)
            {
                throw input_format_error();
            }
            catch (...)
            {
                throw key_type_conflict();
            }
           
        }
        //add it into tuple
        cur_tuple.addKey(key_insert);
        n_insert++;//top++
    }
    //check the insert times
    if (n_insert != attr_find.num)
    {
        std::cout << "insert number of values is wrong\n";
        throw input_format_error();
    }
    if (query[cur_p + 1] == '\0')
    {
        std::cout << "line end unexpectedly\n";
        throw input_format_error();
    }
    //insert the tuple into record
    api.insertRecord(table_name, cur_tuple);
    std::cout << "Insertion Success! \n";
}
    
void Interpreter::decode_delete()
{
    std::string table_name;
    std::string attr_find_name;

    API api;
    CatalogManager catalog_manager;

    Relation relation;
    std::string condition;

    int cur_p;

    if (query.substr(7, 4) != "from")//no from
    {
        std::cout << "no from in delete\n";
        throw 1;
    }

    //get table name
    table_name = fetch_word(12, cur_p);

    if (!catalog_manager.havetable(table_name))//no table found
    {
        std::cout << "no table found by this name\n";
        throw table_not_exist();
    }

    //if delete all in table
    if (query[cur_p + 1] == '\0')
    {
        attr_find_name = "";
        std::vector<Relation> relations_v;
        api.deleteRecord(table_name, relations_v);
        std::cout << "Deletion Success\n";
        return;
    }

    //delete with relation
    //no where
    if (query.substr(cur_p + 1, 5) != "where")
    {
        std::cout << "no where in delete\n";
        throw 1;
    }
    cur_p += 7;
    std::vector<Relation> relations;

    //read in every relation
    while (1)
    {
        //get attribute
        attr_find_name = fetch_word(cur_p, cur_p);
        //chech attribute
        int attribute_num;
        if (!catalog_manager.haveAttribute(table_name, attr_find_name, attribute_num))
        {
            std::cout << "no this attribute in delete\n";
            throw attribute_not_exist();
        }
        //get the condition
        condition = get_condition(cur_p + 1, cur_p);
    
        if (condition == "<") relation.sign = LESS;
        else if (condition == "<=" || condition == "< =") relation.sign = LESS_OR_EQUAL;
        else if (condition == "=") relation.sign = EQUAL;
        else if (condition == ">=" || condition == "> =") relation.sign = GREATER_OR_EQUAL;
        else if (condition == ">") relation.sign = GREATER;
        else if (condition == "<>") relation.sign = NOT_EQUAL;
        else
        {
            std::cout << "not supported relation\n";
            throw 1;
        }

        //get the value and corresponding attribute
        std::string value_delete = fetch_word(cur_p + 1, cur_p);
        Attribute attr_delete = catalog_manager.getAttribute(table_name);

        //do every attribute to find corresponding
        for (int i = 0; i < attr_delete.num; i++)
        {
            if (attr_find_name == attr_delete.name[i])//find corresponding one
            {
                relation.key.type = attr_delete.type[i];
                int cur_type = attr_delete.type[i];
                if (cur_type == -1)//int
                {
                    try
                    {
                        relation.key.INT_VALUE = string2num<int>(value_delete);
                    }
                    catch (...)
                    {
                        std::cout << "int value get failed in delete\n";
                        throw key_type_conflict();
                    }
                    break;
                }
                else if (cur_type == 0)//float
                {
                    try
                    {
                        relation.key.FLOAT_VALUE= string2num<float>(value_delete);
                    }
                    catch (...)
                    {
                        std::cout << "float value get failed in delete\n";
                        throw key_type_conflict();
                    }
                    break;
                }
                else//string
                {
                    try
                    {
                        //if there is no ' or "
                        if (value_delete[0] != '\'' || value_delete[value_delete.length() - 1] != '\'')
                        {
                            if (value_delete[0] != '"' || value_delete[value_delete.length() - 1] != '"')
                            {
                                std::cout << "no \' or \" detected\n";
                                throw input_format_error();
                            }

                        }
                        relation.key.STRING_VALUE = value_delete.substr(1,value_delete.length()-2);
                    }
                    catch (...)
                    {
                        std::cout << "string value get failed in delete\n";
                        throw key_type_conflict();
                    }
                    break;
                }
            }
        }
        //push this relation into it
        relation.attributeName = attr_find_name;
        relations.push_back(relation);

        //read next relation
        if (query[cur_p + 1] == '\0')//end of conditions
        {
            break;
        }
        else if (query.substr(cur_p + 1, 3) != "and")//relations are not ands
        {
            std::cout << "Input format error: no condition \"and\" found\n";
            throw 1;
        }
        fetch_word(cur_p + 1, cur_p);//eat the "and"
        cur_p++;//read in next condition
    }

    
    api.deleteRecord(table_name, relations);
    std::cout << "Deletion Success \n";
}
void Interpreter::decode_exit()
{
    throw exit_command();
}
void Interpreter::decode_file_read()
{
    std::string cur_query;
    std::string::iterator it;
    std::stringstream ss;
    int cur_p = 0;

    std::string path = fetch_word(9, cur_p);

    //more after path
    if (query[cur_p] != '\0')
    {
        std::cout << "more char after path\n";
        throw 1;
    }

    //read file
    std::fstream fs(path);
    ss << fs.rdbuf();
    //get query
    cur_query = ss.str();

    //get and decode every query line in this file
    int i = 0;
    int head = 0;
    do
    {
        while (cur_query[i]!='\n')
        {
            i++;
        }
        query = cur_query.substr(head, i - head);//get the query
        i++;
        head = i;
        split_space();//split with each space
        std::string my_cur_table_name = catch_erro();//decode and catch error
    } while (cur_query[i] != '\0');
}

std::string Interpreter::decode_table_create()
{
    std::string my_cur_table_name = "";
    API api;
    int cur_p = -1;
    std::string table_name = fetch_word(13, cur_p);//get the table name
    current_table_name = table_name;
    my_cur_table_name = table_name;

    Attribute new_attr;
    std::string attr_name;
    Index index_create; 
    int primary = -1;
    int attr_num = 0;

    index_create.indexNumber = 0;
    
    //read it until to the end
    while (1) 
    {
        //check end
        cur_p += 3;
        if (query[cur_p] == '\0') //read to the end
        {
            if (query[cur_p - 2] == '\0')//empty
            {
                std::cout << "input is wriong in table create\n";
                throw 1;
            }
            else break;
        }

        attr_name = fetch_word(cur_p, cur_p);//get the attribute
        std::string is_prim(attr_name);//check if it is primary

        //check primary
        if (is_prim != "primary") //not primary
        {
            new_attr.name[attr_num] = attr_name;
        }
        else//the key may be primary key
        { 
            int p_key = cur_p;
            std::string has_key = fetch_word(p_key + 1, p_key);

            //see is it a key
            if (has_key == "key")//it is primary key 
            {
                cur_p = p_key + 3;//eat the key
                std::string pk_name = fetch_word(cur_p, cur_p);//get the key name

                //check if the name is unique
                int is_find = 1;
                for (int pk_index = 0; pk_index < new_attr.num; pk_index++)
                {
                    if (new_attr.name[pk_index] == pk_name)
                    {
                        is_find = 0;
                        primary = pk_index;
                        new_attr.unique[pk_index] = true;
                        new_attr.primary_key = pk_index;
                        cur_p += 2;
                        break;
                    }
                }
                if (is_find)
                {
                    std::cout << "can not find the primary key when creating table\n";
                    throw 1;
                }
                continue;
            }
            else //not a key, just another attr_name
            {
                new_attr.name[attr_num] = attr_name;
                break;
            }
        }    

        //get attribute
        cur_p++;
        new_attr.type[attr_num] = get_type(cur_p, cur_p);
        new_attr.unique[attr_num] = false;
        
        //check unique
        if (query[cur_p + 1] == 'u' || query[cur_p + 1] == 'U')
        {
            if (fetch_word(cur_p + 1, cur_p) == "unique") 
            {
                new_attr.unique[attr_num] = true;
            }
            else
            {
                std::cout << "unique is wrongly detected\n";
                throw 1;
            }
        }
        attr_num++;
        new_attr.num = attr_num;
    }
    //create table
    api.createTable(table_name, new_attr,index_create);//???
    std::cout << "Create Table Success\n";
    return my_cur_table_name;
}
void Interpreter::decode_table_drop()
{
    int cur_p;
    current_table_name = "";
    
    //get the table name
    std::string table_name = fetch_word(11, cur_p);

    //check end
    if (query[cur_p + 1] != '\0')
    {
        std::cout << "more word after table name\n";
        throw 1;
    }
        
    //drop table
    API api;
    api.dropTable(table_name);
    std::cout << "Drop Table Success" << std::endl;
}
void Interpreter::decode_index_generate()
{
    int cur_p;
    CatalogManager catalog_manager;
    API api;
    
    //get the index name
    std::string index_name = fetch_word(13, cur_p);
    cur_p++;

    if (query.substr(cur_p, 2) != "on")// no on in query
    {
        std::cout << "no on when create index\n";
        throw 1;
    }

    //get the table name
    std::string table_name = fetch_word(cur_p + 3, cur_p);

    if (!catalog_manager.havetable(table_name))
    {
        std::cout << "table name wrong when creating index\n";
        throw table_not_exist();
    }

    if (query[cur_p + 1] != '(')
    {
        std::cout << "no ( when creating index\n";
        throw 1;
    }

    //get the attribute name
    std::string attr_name = fetch_word(cur_p + 3, cur_p);

    if (query[cur_p + 1] != ')' || query[cur_p + 3] != '\0')
    {
        std::cout << "no ) or no end after attribute name when creating index\n";
        throw 1;
    }

    //create index
    api.generate_index(table_name, attr_name, index_name);
    std::cout << "Create Index Success\n";
}
void Interpreter::decode_index_delete()
{
    API api;
    int cur_p;

    //get index name
    std::string index_name = fetch_word(11, cur_p);
    cur_p++;

    std::string table_name = current_table_name;
    //delete the index
    api.delete_index(table_name, index_name);
    std::cout << "Delete Index Success\n";
}




void Interpreter::read_in_command()
{
    std::string cur_s;

    //get query until ";"
    do 
    {
        //read in cur_s
        getline(std::cin, cur_s);
        //add it into query
        query += cur_s;
        query += ' ';
    } while (cur_s.length()==0||cur_s[cur_s.length() - 1] != ';');
    
    query[query.length() - 2] = '\0';
    split_space();
}


std::string Interpreter::decode()
{
    std::string my_cur_table_name = "";
    if (query.substr(0, 6) == "select") 
    {
        decode_select();
    }
    else if (query.substr(0, 4) == "drop") 
    {
        if (query.substr(5, 5) == "table") decode_table_drop();
        else if (query.substr(5, 5) == "index") decode_index_delete();
    }
    else if (query.substr(0, 6) == "insert")
    {
        decode_insert();
    }
    else if (query.substr(0, 6) == "create") 
    {
        if (query.substr(7, 5) == "table") my_cur_table_name = decode_table_create();
        else if (query.substr(7, 5) == "index") decode_index_generate();   
    }
    else if (query.substr(0, 6) == "delete") 
    {
        decode_delete();
    }
    else if (query.substr(0, 8) == "describe" || query.substr(0, 4) == "desc") 
    {
        decode_show();
    }
    else if (query.substr(0, 4) == "exit" && query[5] == '\0') 
    {
        decode_exit();
    }
    else if (query.substr(0, 8) == "execfile") 
    {
        decode_file_read();
    }
    else 
    {
        std::cout << "current query's format is wrong when decoding\n";
        throw input_format_error();
    }
    return my_cur_table_name;
}


//fetch a word and update the tail
std::string Interpreter::fetch_word(int location, int & tail)
{
    std::string tmp = "";
    for(int i=location;i<(int)query.length();i++)
    {
        if(query[i]==' '||query[i]=='\0')
        {
            tmp = query.substr(location,i-location);
            tail = i;
            break;
        }
    }
    return tmp;
}
//get the condition, <=x, !=x
std::string Interpreter::get_condition(int location, int & tail)
{
    std::string ret = "";
    for(int i=location;i<(int)query.length();i++)
    {
        if(query[i]==' ')
        {
            continue;
        }
        if(query[i]!='<' && query[i]!='>' && query[i]!='=' && query[i]!='!')
        {
            ret = query.substr(location,i-location-1);
            tail = i-1;
            break;
        }
    }
    return ret;
}
//get type
int Interpreter::get_type(int location, int& tail)
{
    std::string type_get = fetch_word(location, tail);
    int ret;
    if (type_get == "int") ret = -1;
    else if (type_get == "float") ret = 0;
    else if (type_get == "char") {
        //eat the space
        tail += 3;
        std::string tmp_s = fetch_word(tail, tail);//get the string length
        tail += 2;//eat the space
        ret = atoi(tmp_s.c_str()) + 1;
    }
    else
    {
        std::cout << "the type is not recognized\n";
        throw 1;
    }
    return ret;
}




//return the length of the number with int
int Interpreter::get_len(int number)
{
    int ret = 0;
    if(number==0)
    {
        ret = 1;
        return ret;
    }
    else if(number < 0)
    {
        ret++;
        number = -number;
    }
    while(number != 0)
    {
        number = number/10;
        ret++;
    }
    return ret;
}
//return the length of the number with float
int Interpreter::get_len(float number)
{
    int ret = 0;
    if((int)number==0)
    {
        ret = 4;
        return ret;
    }
    else if(number < 0)
    {
        ret++;
        number = -number;
    }
    int left = number;
    while(left != 0)
    {
        left = left/10;
        ret++;
    }
    return ret+3;
}
//add space to split
void Interpreter::split_space()
{
    int cur_p;
    //read every char in query
    for (cur_p = 0; cur_p < query.length(); cur_p++)
    {
        if (query[cur_p] == '<' || query[cur_p] == '>' || query[cur_p] == '(' || query[cur_p] == ')' || query[cur_p] == '*' || query[cur_p] == ',' || query[cur_p] == '=')
        {
            if (query[cur_p - 1] != ' ')
            {
                query.insert(cur_p, " ");
                cur_p++;
            }
            if (query[cur_p + 1] != ' ')
            {
                query.insert(cur_p + 1, " ");
                cur_p++;
            }
        }
    }
    //add a space at the end
    query.insert(query.length() - 2, " ");
    
    //delete double space or trible space
    int is_one_space = 0;
    std::string::iterator it;
    for (it = query.begin(); it < query.end(); it++)
    {
        if (is_one_space == 0)
        {
            if (*it == ' ' || *it == '\t')//there is space or there is a table
            {
                is_one_space = 1;
                continue;
            }
        }
        else if (*it == ' ' || *it == '\t')//there are double space or double tab
        {
            query.erase(it);
            if (it != query.begin()) it--;
            continue;
        }

        if(*it != ' '&&*it!='\t')//none
        {
            is_one_space = 0;
        }
    }
    //delete head space
    if (query[0] == ' ') query.erase(query.begin());
}
std::string Interpreter::catch_erro()
{
    std::string my_cur_table_name = "";
    try
    {
        my_cur_table_name = decode();
    }
    catch (table_name_conflict error) 
    {
        std::cout << "table has existed!" << std::endl;
    }
    catch (table_not_exist error) 
    {
        std::cout << "table not exist!" << std::endl;
    }
    catch (attribute_not_exist error) 
    {
        std::cout << "attribute not exist!" << std::endl;
    }
    catch (index_exist_conflict error) 
    {
        std::cout << "index has existed!" << std::endl;
    }
    catch (index_not_exist error) 
    {
        std::cout << "index not existed!" << std::endl;
    }
    catch (tuple_type_conflict error) 
    {
        std::cout << "tuple type conflict!" << std::endl;
    }
    catch (primary_key_conflict error) 
    {
        std::cout << "primary key conflict!" << std::endl;
    }
    catch (key_type_conflict error) 
    {
        std::cout << "data type conflict!" << std::endl;
    }
    catch (index_full error) 
    {
        std::cout << "index full!" << std::endl;
    }
    catch (unique_conflict error) 
    {
        std::cout << "unique conflict!" << std::endl;
    }
    catch (exit_command error) 
    {
        std::cout << "good bye" << std::endl;
        exit(0);
    }
    catch (...) {
        std::cout << "input format error!" << std::endl;
    }
    return my_cur_table_name;
}