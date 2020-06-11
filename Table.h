#pragma once

#include "Tuple.h"


struct Attribute{
    int num;  //存放table的属性数
    std::string name[32];  //存放每个属性的名字
    short primary_key;  //判断是否存在主键,-1为不存在，其他则为主键的所在位置
    short type[32];  //存放每个属性的类型，-1：int,0:float,1~255:string的长度+1
    bool unique[32];  //判断每个属性是否unique，是为true
    bool index[32]; //判断是否存在索引
};

//索引管理，一张表最多只能有10个index
struct Index{
    int indexNumber;  //index的总数
    short location[10];  //每个index在Attribute的name数组中是第几个
    std::string indexname[10];  //每个index的名字
};

class Table{
private:
    std::string title;  //表名
    std::vector<Tuple> tuple;  //存放所有的元组
    Index index;  //表的索引信息
public:
    Attribute attribute;  //数据的类型
    //构造函数
    Table() {};
    Table(std::string in_title,Attribute in_attr): title(in_title), attribute(in_attr) { index.indexNumber = 0; };
    Table(const Table &in_table):title(in_table.title), attribute(in_table.attribute), index(in_table.index) {
        for (size_t i=0; i<in_table.tuple.size(); i++)
            tuple.push_back(in_table.tuple[i]);
    };

    std::string getTitle() {return title;};
    Attribute getAttr() {return attribute;};
    std::vector<Tuple>& getTuple() {return tuple;};
    Index getIndex() {return index;};

    bool setIndex(short index,std::string index_name);  //插入索引，输入要建立索引的Attribute的编号，以及索引的名字，成功返回1失败返回0
    bool dropIndex(std::string index_name);  //删除索引，输入建立的索引的名字，成功返回1失败返回0    

    void showTable(); //显示table的部分数据
    void showTable(int limit);
};