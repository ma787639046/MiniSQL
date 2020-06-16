#pragma once

#include "Tuple.h"

/*
    Table.h定义了表的基本结构，一张表的组成如下：
    Table:
        1、表名：title
        2、记录：含多个tuple
                tuple1 = {key1, key2, key3, key4, ……}   （key = type + VALUE(INT, FLOAT, >0(STRING))）
                tuple2 = {key1, key2, key3, key4, ……}
                tuple3 = {key1, key2, key3, key4, ……}
                ……
        3、索引：index
                数量indexNumber、位置location[i]、名字indexname[i]
        4、属性：attribute
                数量num、属性名name[i]、主键位置primary_key、键值类型type[i]、unique[i]、是否存在索引index[i]、
*/

struct Attribute{
    int num;  //属性总数
    std::string name[32];  //属性的名字
    int primary_key;  //主键，-1为不存在主键，其他则为主键的loaction
    int type[32];  //属性的类型
    bool unique[32];  //属性是否unique
    bool index[32]; //是否存在索引
};

struct Index{
    int indexNumber;  //index的总数
    int location[32];  //index在attribute中的位置，就是序号
    std::string indexname[32];  //index的名字
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