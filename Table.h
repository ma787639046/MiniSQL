//
//	Table.h
//	Created by MGY on 2020/06/04
//

/*
	Table.h定义了表的基本数据结构
*/

/*
	Table的构成如下：
		一个Table：
			TableName
			Attribute：int，float，string……
			Record: Tuple1 (1, 2.1, "ABC" ……)
					Tuple2 (2, 1.1, "BC" ……)
					……
			Index:(IndexName, 对应第几个Attribute)
*/
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

/* 下面定义了Attribute, Attribute最多有32个 */
typedef enum {
	INT,
	FLOAT,
	STRING
} AttrType;

//class keyAttr {
//public:
//	keyAttr() :name(""), isUnique(false), hasIndex(false) {};	//默认构造函数
//	std::string name;	//存放每个属性的名字
//	AttrType type;	//存放每个属性的类型
//	size_t maxStringSize;	//如果type = STRING，则最大的string长度是多少
//	bool isUnique;	//判断每个属性是否unique，是为true
//	bool hasIndex;	//对应属性值是否存在索引
//};
//
//class Attribute {
//public:
//	Attribute();
//	Attribute(const Attribute& attr);	//拷贝构造函数
//	int primary_key;	//判断是否存在主键,-1为不存在，其他则为主键的所在位置
//	std::vector<keyAttr> attr;	//储存每个key的attribute信息
//};


class Attribute {
public:
	Attribute();
	Attribute(const Attribute& attr);	//拷贝构造函数
	int attributeNumber;	//attribute个数
	AttrType type[32];	//存放每个属性的类型
	std::string name[32];	//存放每个属性的名字
	bool unique[32];	//判断每个属性是否unique，是为true
	int primary_key;	//判断是否存在主键,-1为不存在，其他则为主键的所在位置
	bool index[32];	//对应属性值是否存在索引
	size_t maxStringSize[32];	//如果type = STRING，则最大的string长度是多少
};

/* 下面定义了Index */
class Index {
public:
	Index();
	Index(const Index& index);
	int indexNumber;	//index的个数
	int num[32];	//每个index在Attribute的name数组中是第几个
	std::string indexname[32]; //每个index的名字
};

/* 下面定义了Tuple，record由多条Tuple构成 */
//struct定义一个key，Tuple = (key1, key2, key3 ……)
//使用key之前，必须先通过Attribute.type[i]确定key是int，float，还是string
//：：由于union中无法使用string，而直接用char[255]太浪费空间，因此用struct嵌套
//：：key，比union更加省空间
class key_ {
public:
	int INT_VALUE;
	float FLOAT_VALUE;
	std::string STRING_VALUE;
};

//	定义一条Tuple，Tuple = (key1, key2, key3 ……)
class Tuple {
public:
	Tuple() :valid(false) {};	//默认构造函数，valid=false
	Tuple(const Tuple& tuple);  //拷贝构造函数
	void addTuple(key_ tuple);	//向这条Tuple中push_back一个值
	std::vector<key_>& getTuple();	//返回Tuple的引用
	void showTuple(Attribute& attribute);	//显示Tuple的所有数据，需要给定对应的Attribute 
	int size();	//返回Tuple总数
	bool getValid();	//返回Tuple是否可用
	void setValid(bool status);	//设置Tuple是否可用
	std::string encodeTuple();	//将Tuple转换为string形式，方便储存。
								//Tuple = 4bytes的Tuple size:n + (4 bytes INT + 4 bytes FLOAT + 4 bytes sizeof(STRING_VALUE) + STRING_VALUE)*n
	void decodeTuple(std::string& string);	//将string形式的Tuple解码，剩余的string存回，用于连续使用decodeTuple
private:
	std::vector<key_> tuple;	//一条tuple，Tuple = (key1, key2, key3 ……) 
	bool valid;	//	定义Tuple是否可用
};

/* 下面定义Table，Table包含TableName，Attribute，Record，Index*/
class Table {
public:
	std::string TableName;	//表名
	Attribute attribute;	//表的属性
	std::vector<Tuple> Record;	//表的记录，包含多条Tuple
	Index index;	//表的索引信息

	Table() {};	//默认构造函数
	Table(const Table& table);	//拷贝构造函数
	Table(std::string TableName, Attribute attribute);	//由TableName和attribute建立空表

	bool setIndex(int index, std::string index_name);	//插入索引，输入要建立索引的Attribute的编号，以及索引的名字，成功返回true,失败返回false
	bool dropIndex(std::string index_name);	//删除索引，输入建立的索引的名字，成功返回true,失败返回false

	void showTable(); //显示table数据
	void showTable(int limit);	//显示至多limit行数据
};