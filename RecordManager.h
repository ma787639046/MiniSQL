#pragma once

#include <sstream>
#include "BufferManager.h"
#include "Table.h"
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
	
	std::vector<Tuple> loadTuples(std::string table_name);	//将table_name中所有的tuples装载至这个vector
private:
	void setBlockStringSize(int size, std::string& s);		//设置block的已用空间大小
	void setTupleStringSize(int size, std::string& s);		//设置Tuple的个数
	std::string createFormatedBlockString();	//创建一个可以刷入block的string
	int getBlockStringSize(std::string s);	//读取BlockString的已用空间的总大小
	int getTupleStringSize(std::string s);	//获取Block中Tuple的个数
	void writeTupleString(std::string& s, Tuple& tuple);	//向s中写入一个Tuple
	std::vector<Tuple> readTupleString(std::string& s);	//读取Tuple至一个vector中

	void flushToBlock(std::string s, int block_id, std::string table_name);	//将Record记录刷入磁盘文件对应的block中
	std::string loadBlockString(int block_id, std::string table_name);	//将一个block装载至string中
};