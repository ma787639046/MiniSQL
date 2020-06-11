/*
	Catalog中存放索引记录Index，Catalog只负责说明哪些attribute有或需要建index
	建索引的操作，需要调用index manager的接口。
*/
#pragma once

#include <iostream>
#include "Table.h"
#include "BufferManager.h"
#include "const.h"
#include "exceptions.h"

class CatalogManager {
public:
	//createTable(): 创建表
	//输入：表名，属性，索引
	//异常：如果存在表名相同的表，则抛出table_name_conflict异常 
	void createTable(std::string table_name, Attribute attribute, int primary, Index index);

	//havetable()：判断是否有名称为table_name的table存在
	//输入：表名
	//输出：true，false
	bool havetable(std::string table_name);

	//dropTable()：删除表
	//输入：表名
	//异常：表不存在，抛出table_not_exist
	void dropTable(std::string table_name);

	//haveAttribute():判断名为table_name的表，是否存在名为attribute_name的属性
	//				  并将找到的attribute的序号写入attribute_num中
	//异常：表不存在，抛出table_not_exist；
	bool haveAttribute(std::string table_name, std::string attribute_name, int &attribute_num);

	//getAttribute()：获取名为table_name的表的属性
	//异常：表不存在，抛出table_not_exist；
	Attribute getAttribute(std::string table_name);

	//addIndex()：向名为table_name的表，为名为attribute_name的属性添加名为index_name的索引记录
	//异常：1，表不存在，抛出table_not_exist；2，属性不存在，抛出attribute_not_exist；
	//		3，属性已经有index，抛出index_exist_conflict；4，index个数超出最大值，抛出index_full（index num设置为32，所以超不了）；
	//		5，索引名存在，抛出index_name_conflict; 6,属性不unique，抛出attribute_not_unique
	void addIndex(std::string table_name, std::string attribute_name, std::string index_name);

	//dropIndex()：在名为table_name的表内，删除名为index_name的索引
	//输入：表名、索引名
	//异常：1，表不存在，抛出table_not_exist；2，索引不存在，抛出index_not_exist
	void dropIndex(std::string table_name, std::string index_name);

	//findAttributeThroughIndex()：在名为table_name的表内，查找index_name所对应的属性的属性名
	//输入：表名、索引名
	//输出：属性名
	//异常：1，表不存在，抛出table_not_exist；2，索引不存在，抛出index_not_exist
	std::string findAttributeThroughIndex(std::string table_name, std::string index_name);

    //showCatalog()：打印名为table_name的表的表名、属性、索引信息
	//异常：表不存在，抛出table_not_exist
	void showCatalog(std::string table_name);

private:
    void getTableBlock(std::string name, int& block_id, int& offset);  //返回表所在的块的块号和偏移量
	int getBlockNumber(std::string table_name); //获取文件有多少个块
    std::string decodeTableName(char* buffer, int start, int& end); //获取buffer中，从start开始的表名，并将最终的偏移量传递给end
    //getIndex()：获取名为table_name的表的索引信息（哪些属性有索引）
	//异常：表不存在，抛出table_not_exist；
    Index getIndex(std::string table_name); //从catalog中获得索引
    std::string encodeNum(int num, short width);    //将num转化为width宽的string
};

extern BufferManager buffer_manager;