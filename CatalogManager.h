//
//	CatalogManager.h
//	Created by MGY on 2020/06/04
//

/*
	Catalog中存放索引记录Index，Catalog只负责说明哪些attribute有或需要建index
	建索引的操作，需要调用index manager的接口。
*/
#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include "Table.h"
#include "BufferManager.h"
#include "const.h"
#include "exceptions.h"
//for xcode
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

extern BufferManager buffer_manager;

/*
	为方便CatalogManager通过BufferManager管理block内的各个table，
	建立Catalog类，以直接容纳TableName，attribute，index。
	Catalog类相当于Table类的一个极度的简化版，主要用于CatalogManager
	内部的数据管理。
*/
class Catalog {
public:
	std::string TableName;	//表名
	Attribute attribute;	//表的属性
	Index index;	//表的索引信息
};

class CatalogManager {
public:
	CatalogManager();

	//createTable(): 创建表
	//输入：表名，属性，索引
	//异常：如果存在表名相同的表，则抛出table_name_conflict异常 
	void createTable(std::string table_name, Attribute attribute, Index index);

	//havetable()：判断是否有名称为table_name的table存在
	//输入：表名
	//输出：true，false
	bool havetable(std::string table_name);

	//findTableBlock(): 找到并返回Table所在的block_id
	//输入：表名
	//输出：Table所在的block_id，没有则返回-1	（功能上是havetable的强化版）
	int findTableBlock(std::string table_name);

	//dropTable()：删除表
	//输入：表名
	//异常：表不存在，抛出table_not_exist
	void dropTable(std::string table_name);

	//getCatalog()：获取名为table_name的表的表名、属性、索引信息，存放在Catalog对象中
	//输入：表名
	//异常：表不存在，抛出table_not_exist
	Catalog getCatalog(std::string table_name);

	//showCatalog()：打印名为table_name的表的表名、属性、索引信息
	//异常：表不存在，抛出table_not_exist
	void showCatalog(std::string table_name);

	//haveAttribute():判断名为table_name的表，是否存在名为attribute_name的属性
	//				  并将找到的attribute的序号写入attribute_num中
	//异常：表不存在，抛出table_not_exist；
	bool haveAttribute(std::string table_name, std::string attribute_name, int& attribute_num);

	//getAttribute()：获取名为table_name的表的属性
	//异常：表不存在，抛出table_not_exist；
	Attribute getAttribute(std::string table_name);

	//getIndex()：获取名为table_name的表的索引信息（哪些属性有索引）
	//异常：表不存在，抛出table_not_exist；
	Index getIndex(std::string table_name);

	//addIndex()：向名为table_name的表，为名为attribute_name的属性添加名为index_name的索引记录
	//异常：1，表不存在，抛出table_not_exist；2，属性不存在，抛出attribute_not_exist；
	//		3，属性已经有index，抛出index_exist_conflict；4，index个数超出最大值，抛出index_full（index num设置为32，所以超不了）；
	//		5，索引名存在，抛出index_name_conflict; 6,属性不unique，抛出attribute_not_unique
	void addIndex(std::string table_name, std::string attribute_name, std::string index_name);

	//dropIndex()：在名为table_name的表内，删除名为index_name的索引
	//输入：表名、索引名
	//异常：1，表不存在，抛出table_not_exist；2，索引不存在，抛出index_not_exist
	void dropIndex(std::string table_name, std::string index_name);
private:
	//创建表Record文件
	void createCatalogFile();	

	//创建一个可以刷入block的string
	std::string createFormatedBlockString();

	//获取文件有多少个块
	int getBlockNumber();

	//设置文件的块数
	void setBlockNumber(int block_number);

	//设置block的已用空间大小
	void setBlockStringSize(int size, char* pointer);

	//设置Table的个数
	void setTableStringSize(int size, char* pointer);

	//读取BlockString的已用空间的总大小
	int getBlockStringSize(char* pointer);

	//获取Block中Table的个数
	int getTableNum(char* pointer);

	//向pointer中写入一个Table
	void writeTableString(char* pointer, std::string& encoded_tuple);

	//encodeCatalog()：将Catalog编码为可以写入的string
	//输入：catalog对象
	//返回：encoded string
	std::string encodeCatalog(Catalog& catalog);

	//decodeCatalog()：将page_pointer开始，offset偏移量开始的catalog解码为Catalog对象
	//输入：页指针，偏移量offset的引用（指向catalog的地址）
	//返回：catalog对象
	Catalog decodeSingleCatalog(char* page_pointer, int& offset);

	//连续调用decodeSingleCatalog()，将获得的所有Catalog储存至数组中返回
	std::vector<Catalog> decodeAllCatalog(char* pointer);

	//判断文件是否存在
	bool exists_file(const char* name);
};

inline bool CatalogManager::exists_file(const char* name) {
	struct stat buffer;
	return (stat(name, &buffer) == 0);
}
