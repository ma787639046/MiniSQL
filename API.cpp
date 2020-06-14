#include "API.h"


#include <time.h>

//testing
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
	api.generate_index(table_name, attribute.name[0], "index0", attribute.type);
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



int API::generate_index(std::string table_name, std::string attribute_name, std::string index_name, int* string_sizes)//generate an index based on the attribute of the table
{
	int find_attr_flag = 0;
	int find_type = 0;
	//get the path
	std::string path = "INDEX_FILE_" + attribute_name + "_" + table_name;
	//create index manager
	IndexManager index_manager(table_name);
	//get catalog manager
	catalog_manager.addIndex(table_name, attribute_name, index_name);
	Attribute attr_tmp = catalog_manager.getAttribute(table_name);
	int i;
	for (i = 0; i < attr_tmp.num; i++)
	{
		if (attr_tmp.name[i] == attribute_name)
		{
			find_attr_flag = 1;
			break;
		}
	}
	if (find_attr_flag == 1)
	{
		find_type = (int)attr_tmp.type[i];
		index_manager.generate_index(path, find_type, find_type);
		record_manager.generate_index(index_manager, table_name, attribute_name);
		return 1;
	}
	else
	{
		std::cout << "The name of attribute not found!\n";
		return 0;
	}
}

int API::delete_index(std::string table_name, std::string index_name)//delete the index
{
	int is_found = 0;
	//get the path
	std::string attribute_name = catalog_manager.IndextoAttr(table_name, index_name);
	std::string path = "INDEX_FILE_" + attribute_name + "_" + table_name;
	//get index manager
	IndexManager index_manager(table_name);
	Attribute attr_tmp = catalog_manager.getAttribute(table_name);
	int i;
	for (i = 0; i < attr_tmp.num; i++)
	{
		if (attr_tmp.name[i] == attribute_name)
		{
			is_found = 1;
			break;
		}
	}
	if (is_found == 1)
	{
		index_manager.drop_index(path, (int)attr_tmp.type[i]);
		catalog_manager.dropIndex(table_name, index_name);
		path += INDEX_PATH;
		remove(path.c_str());//make path from string to char *, then remove
		return 1;
	}
	else
	{
		std::cout << "The name of attribute not found!\n";
		return 0;
	}
}


