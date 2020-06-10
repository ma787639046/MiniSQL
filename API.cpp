//
//	API.cpp
//	Created by MGY on 2020/06/09
//
#include "API.h"
#include "IndexManager.h"
#include "const.h"



void API::createTable(std::string table_name, Attribute attribute, Index index)
{
	catalog_manager.createTable(table_name, attribute, index);
	record_manager.createTableFile(table_name);
}

void API::dropTable(std::string table_name)
{
	catalog_manager.dropTable(table_name);
	record_manager.dropTableFile(table_name);
}

Table API::selectRecord(std::string table_name, std::vector<Relation> relation)
{
	Table table(table_name, catalog_manager.getAttribute(table_name));	//建立空表
	table.index = catalog_manager.getIndex(table_name);	//添加索引信息至table对象
	std::vector<Tuple> tuple = record_manager.loadRecord(table_name, relation);	//获得所有符合relation的记录
	for (size_t i = 0; i < tuple.size(); i++)
		table.Record.push_back(tuple[i]);	//将记录添加至table对象
	return table;
}

int API::deleteRecord(std::string table_name, std::vector<Relation> relation)
{
	return record_manager.deleteRecord(table_name, relation);
}

void API::insertRecord(std::string table_name, Tuple& tuple)
{
	record_manager.insertRecord(table_name, tuple);
}



int API::generate_index(std::string table_name, std::string attribute_name, std::string index_name, int * string_sizes)//generate an index based on the attribute of the table
{
    int find_attr_flag = 0;
    //get the path
    std::string path = "INDEX_FILE_"+attribute_name+"_"+table_name;
    //create index manager
    IndexManager index_manager(table_name,string_sizes);
    //get catalog manager
    catalog_manager.addIndex(table_name, attribute_name, index_name);
    Attribute attr_tmp = catalog_manager.getAttribute(table_name);
    int i;
    for(i=0;i<attr_tmp.attributeNumber;i++)
    {
        if(attr_tmp.name[i]==attribute_name)
        {
            find_attr_flag = 1;
            break;
        }
    }
    if(find_attr_flag==1)
    {
        index_manager.generate_index(path, (int)attr_tmp.type[i], string_sizes[i]);
        //TODO
        record_manager.generate_index(index_manager,table_name,attribute_name);
        return 1;
    }
    else
    {
        std::cout<<"The name of attribute not found!\n";
        return 0;
    }
}

int API::delete_index(std::string table_name, std::string index_name, int * string_sizes)//delete the index
{
    int is_found = 0;
    //get the path
    std::string attribute_name = catalog_manager.findAttributeThroughIndex(table_name, index_name);
    std::string path = "INDEX_FILE_"+attribute_name+"_"+table_name;
    //get index manager
    IndexManager index_manager(table_name,string_sizes);
    Attribute attr_tmp = catalog_manager.getAttribute(table_name);
    int i;
    for(i=0;i<attr_tmp.attributeNumber;i++)
    {
        if(attr_tmp.name[i] == attribute_name)
        {
            is_found = 1;
            break;
        }
    }
    if(is_found==1)
    {
        index_manager.drop_index(path, (int)attr_tmp.type[i]);
        catalog_manager.dropIndex(table_name, index_name);
        path += INDEX_PATH;
        remove(path.c_str());//make path from string to char *, then remove
        return 1;
    }
    else
    {
        std::cout<<"The name of attribute not found!\n";
        return 0;
    }
}
