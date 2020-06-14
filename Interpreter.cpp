#include "Interpreter.h"


Interpreter::Interpreter(){}
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
        std::cout<<"No such table name\n";
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
                if(tmp_cur_attr_name == tmp_attr.name[i])//find the attribute
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
    
    //get the max attr_key length
    int length_max = -5;
    for(int i=0;i<attribute_name.size();i++)
    {
        if((int)current_attr.name[attr_pointer[i]].length() > length_max)
        {
            length_max = (int)current_attr.name[attr_pointer[i]].length();
        }
    }
    for(int index=0;index<attribute_name.size();index++)
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
                if(length_max < get_len((int)ret_tuple[i].getKeys()[attr_pointer[index]].STRING_VALUE.length()))
                {
                    length_max = get_len((int)ret_tuple[i].getKeys()[attr_pointer[index]].STRING_VALUE.length());
                }
            }
        }
    }
    length_max++;
    
    //for each attribute
    for(int index=0;index<attribute_name.size();index++)
    {
        //for i < (max_lenth-current_length)/2, left part
        for(int i=0;i<(length_max-current_attr.name[attr_pointer[index]].length())/2;i++)
        {
            std::cout<<" ";
        }
        std::cout<<current_attr.name[attr_pointer[index]].c_str();
        //for i < (max_lenth-current_length)/2, right part
        for(int i=0;i<current_attr.name[attr_pointer[index]].length()/2;i++)
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
//return the length of the number
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
