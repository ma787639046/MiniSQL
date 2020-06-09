//
//	API.cpp
//	Created by MGY on 2020/06/09
//
#include "API.h"

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
