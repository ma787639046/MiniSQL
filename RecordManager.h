#pragma once

#include <sstream>
#include "BufferManager.h"
#include "CatalogManager.h"
#include "Table.h"
#include "Relation.h"
//	std::vector<key_> tuple;	//一条tuple，Tuple = (key1, key2, key3 ……) 
//	std::vector<Tuple> Record;	//表的记录，包含多条Tuple

//Record Manager负责管理记录表中数据的数据文件。主要功能为实现数据文件的创建与删除（由表的定义与
//删除引起）、记录的插入、删除与查找操作，并对外提供相应的接口。其中记录的查找操作要求能够支持不带
//条件的查找和带一个条件的查找（包括等值查找、不等值查找和区间查找）。`

//数据文件由一个或多个数据块组成，块大小应与缓冲区块大小相同。一个块中包含一条至多条记录，为简单起
//见，只要求支持定长记录的存储，且不要求支持记录的跨块存储。



class RecordManager {
public:
	void createTableFile(std::string table_name);	//创建表Record文件
	void dropTableFile(std::string table_name);		//删除表Record文件
	
	// deleteRecord(std::string table_name)：删除名为table_name表中的所有Record（不删除文件，文件需调用dropTableFile()删除）
	// 输入：表名
	// 输出：删除记录的条数
	// 异常处理：1、table_name对应的表名在catalog中不存在，抛出table_not_exist
	int deleteRecord(std::string table_name);

	// deleteRecord(std::string table_name, std::vector<Relation> relation)：删除名为table_name表中的所有符合relation关系的Record
	// 输入：表名，关系
	// 输出：删除记录的条数
	// 异常处理：1、table_name对应的表名在catalog中不存在，抛出table_not_exist
	//			2、Attribute的类型与输入的relation 不匹配，抛出key_type_conflict
	int deleteRecord(std::string table_name, std::vector<Relation> relation);

	// insertRecord()：向名为table_name中的表格，插入一条tuple记录
	// 输入：表名，tuple
	// 输出：无
	// 异常处理：1、table_name对应的表名在catalog中不存在，抛出table_not_exist
	//			2、插入重复的主键，抛出primary_key_conflict
	//			3、属性unique，插入重复的值，抛出unique_conflict异常
	void insertRecord(std::string table_name, Tuple& tuple);

	// loadRecord(std::string table_name): 将table_name中所有的tuples装载至这个vector
	// 输入：表名
	// 输出：vector<Tuple>
	// 异常处理：1、table_name对应的表名在catalog中不存在，抛出table_not_exist
	std::vector<Tuple> loadRecord(std::string table_name);	//将table_name中所有的tuples装载至这个vector

	// loadRecord(std::string table_name, std::vector<Relation> relation): 将table_name中所有满足relation关系
	//																		的tuples装载至这个vector
	// 输入：表名、关系数组vector<Relation>
	// 输出：vector<Tuple>
	// 异常处理：1、table_name对应的表名在catalog中不存在，抛出table_not_exist
	//			2、relation中，名为Attribute name的Attribute不存在，则抛出attribute_not_exist
	//			3、Attribute的类型与输入的relation 不匹配，抛出key_type_conflict
	std::vector<Tuple> loadRecord(std::string table_name, std::vector<Relation> relation);	//将table_name中所有满足relation关系的tuples装载至这个vector
private:
	void setBlockStringSize(int size, std::string& s);		//设置block的已用空间大小
	void setTupleStringSize(int size, std::string& s);		//设置Tuple的个数
	std::string createFormatedBlockString();	//创建一个可以刷入block的string
	int getBlockStringSize(std::string s);	//读取BlockString的已用空间的总大小
	int getTupleNum(std::string s);	//获取Block中Tuple的个数
	void writeTupleString(std::string& s, Tuple& tuple);	//向s中写入一个Tuple
	int getTupleSize(Tuple tuple);	//返回一个tuple encode为string后的大小
	std::vector<Tuple> decodeTupleString(const std::string s);	//将所有的Tuple读取至一个vector中
	bool meetRelation(key_ key, Relation relation);	//判断key是否满足单条relation关系，是返回true，否返回false

	void flushToBlock(std::string s, int block_id, std::string table_name);	//将Record记录刷入对应的page中
	std::string loadBlockString(int block_id, std::string table_name);	//将一个block装载至string中
};

/*
RecordManager工作原理：
1、RecordManager以page/block为单位进行读取、写入操作。
2、数据储存格式为：4 bytes的已用空间 + 4 bytes的tuple个数 + n个tuple
3、tuple的存储格式为：4bytes的Tuple size:n + (4 bytes INT + 4 bytes FLOAT + 4 bytes sizeof(STRING_VALUE) + STRING_VALUE)*n
RecordManager读取、写入tuple原理：
写入tuple：
1、利用loadBlockString()将磁盘文件中的一个block装入FormatedBlockString中，此时PAGESIZE大小的block数据全部在这个string
2、利用getBlockStringSize()获取block已用空间，用getTupleSize()获取新tuple的空间。如果两者之和小于PAGESIZE，则
   调用writeTupleString()向block写入tuple，然后调用flushToBlock()将block刷入对应的page
读取tuple：
1、利用loadBlockString()将磁盘文件中的一个block装入FormatedBlockString中，此时PAGESIZE大小的block数据全部在这个string
2、利用decodeTupleString()将所有的Tuple读取至一个vector中

示例代码：
1、遍历所有block，获得每个block里面的tuple：
	BufferManager* bufffer_manager = new BufferManager(table_path, 1);	//因为只需获取block number，所以无需较多page
	int blocknum = bufffer_manager->getBlockNum();
	delete(bufffer_manager);    //获取一共有几个block需要遍历
	std::string formatedString;
	formatedString.reserve(PAGESIZE);   //formatedString是一个block中的原始信息，就是把block中的char[PAGESIZE]的所有东西都原封不动读进来
	for (size_t i = 0; i < blocknum; i++) {
		formatedString = loadBlockString(i, table_name);
		std::vector<Tuple> tuple = decodeTupleString(formatedString);   //decodeTupleString将原始数据decode为多个tuple，这时你就得到这个block有哪些tuple了。一条tuple = key1, key2, key3, ……
		**这里干自己想要做的事情**
		tuple.clear();	//清空tuple
	}

*/