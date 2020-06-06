#pragma once

#include <iostream>
#include "Table.h"
#include "BufferManager.h"
#include "const.h"
#include "exceptions.h"

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

namespace catalog {
	BufferManager buffer(CATALOG_FILEPATH, 256);	//文件路径CATALOG_FILEPATH，缓冲区预留256个页（页可动态扩充）
}

class CatalogManager {
public:
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
	//异常：表不存在，抛出table_not_exist；
	bool haveAttribute(std::string table_name, std::string attribute_name);

	//getAttribute()：获取名为table_name的表的属性
	//异常：表不存在，抛出table_not_exist；
	Attribute getAttribute(std::string table_name);

	//addIndex()：向名为table_name的表，为名为attribute_name的属性添加名为index_name的索引记录
	//异常：1，表不存在，抛出table_not_exist；2，属性不存在，抛出attribute_not_exist；
	//		3，属性已经有index，抛出index_exist_conflict；4，index个数超出最大值，抛出index_full（index num设置为32，所以超不了）；
	//		5，索引名存在，抛出index_name_conflict; 6,属性不unique，抛出attribute_not_unique
	void addIndex(std::string table_name, std::string attribute_name, std::string index_name);

	//findAttributeThroughIndex()：在名为table_name的表内，查找index_name所对应的属性的属性名
	//输入：表名、索引名
	//输出：属性名
	//异常：1，表不存在，抛出table_not_exist；2，索引不存在，抛出index_not_exist
	std::string findAttributeThroughIndex(std::string table_name, std::string index_name);

	//dropIndex()：在名为table_name的表内，删除名为index_name的索引
	//输入：表名、索引名
	//异常：1，表不存在，抛出table_not_exist；2，索引不存在，抛出index_not_exist
	void dropIndex(std::string table_name, std::string index_name);

	//loadCatalogFromPage()：从页号为page_id的页，加载Catalog至一个vector
	//输入：页号
	//返回：vector<Catalog>
	std::vector<Catalog> loadCatalogFromPage(int page_id);

	//catalogSize()：测量catalog的总大小，以判断这个page能否有足够空间写入新的catalog信息
	//输入：<vector>catalog
	//返回：catalog写入时的总大小
	int catalogSize(std::vector<Catalog>& catalog);

	//flushCatalogToPage()：将catalog写入page_id页，使用前注意catalog的总大小
	//输入：catalog，page_id
	void flushCatalogToPage(std::vector<Catalog>& catalog, int page_id);
};

