//
//	API.cpp
//	Created by MGY on 2020/06/09
//
#include "API.h"

#include <time.h> 

//测试
int main() {
	clock_t start_time = clock();

	//创建一个新的表
	std::string table_name = "Library";
	Attribute attribute;
	attribute.num = 3;
	attribute.name[0] = "bno";		attribute.name[1] = "title";	attribute.name[2] = "price";
	attribute.type[0] = INT;		attribute.type[1] = 7;		attribute.type[2] = FLOAT;
	attribute.primary_key = 0;	//没有primary key
	attribute.unique[0] = true;	attribute.unique[1] = false;	attribute.unique[2] = false;
	attribute.index[0] = true;		attribute.index[1] = false;		attribute.index[2] = false;
	Index index;
	index.indexNumber = 1;
	index.location[0] = 0;
	index.indexname[0] = "bno_index";

	//设置一条tuple
	key_ key;
	key.INT_VALUE = 1;
	key.STRING_VALUE = "Github";
	key.FLOAT_VALUE = 3.1;
	Tuple tuple;
	key.type = INT;
	tuple.addKey(key);
	key.type = (keyType)7;
	tuple.addKey(key);	
	key.type = FLOAT;
	tuple.addKey(key);

	//设置空relation
	std::vector<Relation> relation;
	
	//开始测试
	API api;
	api.createTable(table_name, attribute, index);
	//api.dropTable(table_name);
	//api.showAttributeInfo(table_name);

	//for (size_t i = 0; i < 1000; i++) {
		//api.insertRecord(table_name, tuple);
	//}
	//std::cout << "Delete " << api.deleteRecord(table_name, relation) << " records.\n";

	//CatalogManager catalog_manager;
	//catalog_manager.addIndex(table_name, "bno", "bno_index");
	//api.showAttributeInfo(table_name);

	//Table table = api.selectRecord(table_name, relation);
	//table.showTable();

	

	clock_t end_time = clock();
	std::cout << std::endl;
	std::cout << "Processed in " << end_time - start_time << " ms" << std::endl;
	getchar();
	return 0;
}


void API::createTable(std::string table_name, Attribute attribute, Index index)
{
	catalog_manager.createTable(table_name, attribute,index);
	record_manager.createTableFile(table_name);
}

void API::dropTable(std::string table_name)
{
	catalog_manager.dropTable(table_name);
	record_manager.dropTableFile(table_name);
}

Table API::selectRecord(std::string table_name, std::vector<Relation> relation)
{
	if (relation.size() == 0) return record_manager.loadRecord(table_name);
	else return record_manager.loadRecord(table_name, relation);
}

int API::deleteRecord(std::string table_name, std::vector<Relation> relation)
{
	if (relation.size() == 0) return record_manager.deleteRecord(table_name);
	return record_manager.deleteRecord(table_name, relation);
}

void API::insertRecord(std::string table_name, Tuple tuple)
{
	record_manager.insertRecord(table_name, tuple);
}

void API::showAttributeInfo(std::string table_name)
{
	catalog_manager.showCatalog(table_name);
}


