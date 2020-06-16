#pragma once

#include "Tuple.h"

struct Attribute{
    int num;  //存放table的属性总数
    std::string name[32];  //存放每个属性的名字
    int primary_key;  //判断是否存在主键，-1为不存在，其他则为主键的所在位置
    int type[32];  //存放每个属性的类型
    bool unique[32];  //判断每个属性是否unique，是为true
    bool index[32]; //判断是否存在索引
};

struct Index{
    int indexNumber;  //index的总数
    int location[32];  //每个index在Attribute的name数组中是第几个
    std::string indexname[32];  //每个index的名字
};

class Table{
private:
    std::string title;  //表名
    std::vector<Tuple> tuple;  //记录
    Index index;  //索引
public:
    Attribute attribute;  //属性
    Table() {};
    Table(std::string in_title,Attribute in_attr): title(in_title), attribute(in_attr) { index.indexNumber = 0; };
    Table(const Table &in_table):title(in_table.title), attribute(in_table.attribute), index(in_table.index) {
        for (size_t i=0; i<in_table.tuple.size(); i++)
            tuple.push_back(in_table.tuple[i]);
    };
    std::vector<Tuple>& getTuple() {return tuple;};
    void showTable();
    void showTable(int limit);
};