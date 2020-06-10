//
//	API.h
//	Created by MGY on 2020/06/09
//

/*
	API：封装底层的3个Manager的各函数，提供接口给顶层Interpreter使用
		API不产生异常，异常的产生由底层Manager进行；
		API不处理异常，异常的处理由顶层Interpreter进行。
*/
#pragma once

#include "Table.h"
#include "RecordManager.h"
#include "CatalogManager.h"

class API {
private:
    CatalogManager catalog_manager;
    RecordManager record_manager;
public:
    API();
    ~API();
	//createTable(): 创建表
	//输入：表名，
	//		属性：个数、名称、类型、是否unique、主键（没有主键设为-1，有主键写对应下标。
	//												注意主键一定是unique的）、是否有index，
	//		索引：个数，索引名，索引对应第几个属性(主键一定有索引)
	//异常：如果存在表名相同的表，则抛出table_name_conflict异常 
	void createTable(std::string table_name, Attribute attribute, Index index);

	//dropTable()：删除表
	//输入：表名
	//异常：表不存在，抛出table_not_exist
	void dropTable(std::string table_name);

	//selectRecord()：查找与relation相符的记录，并返回带有记录的表
	//输入：表名、关系数组：关系1 and 关系2 and 关系3 ……
	//						一个关系 = 属性名 + 属性类型（INT/FLOAT/STRING）+ 符号（>, >=, != ……）+ 值
	//						例如：relation1 = "salary", FLOAT, GREATER_OR_EQUAL, 3000.0f
	//输出：带有记录的表
	//异常：1、table_name对应的表名在catalog中不存在，抛出table_not_exist
	//		2、relation中，名为Attribute name的Attribute不存在，则抛出attribute_not_exist
	//		3、Attribute的类型与输入的relation 不匹配，抛出key_type_conflict
	Table selectRecord(std::string table_name, std::vector<Relation> relation);

	//deleteRecord()：删除表中符合relation的记录，并返回删除的条数
	//输入：表名、关系数组：关系1 and 关系2 and 关系3 ……
	//输出：删除的条数
	//异常：1、table_name对应的表名在catalog中不存在，抛出table_not_exist
	//		2、Attribute的类型与输入的relation 不匹配，抛出key_type_conflict
	int deleteRecord(std::string table_name, std::vector<Relation> relation);

	//insertRecord()：向名为table_name中的表格，插入一条tuple记录
	//输入：表名，tuple
	//输出：无
	//异常处理：1、table_name对应的表名在catalog中不存在，抛出table_not_exist
	//			2、插入重复的主键，抛出primary_key_conflict
	//			3、属性unique，插入重复的值，抛出unique_conflict异常
	//注意：这个函数不会检查tuple类型、个数是否与Attribute匹配。检查请务必放在Interpreter执行！！！！
	void insertRecord(std::string table_name, Tuple& tuple);

    int generate_index(std::string table_name, std::string attribute_name, std::string index_name, int * string_sizes);//generate an index based on the attribute of the table
    int delete_index(std::string table_name, std::string index_name, int * string_sizes);//delete the index

};
