//
//  Relation.h
//  Created by MGY on 2020.06.08
//

/*
     Relation.h用于存储Where条件查找的关系和数值
     API在调用RecordManager进行Where条件查找时，可以传入一个~多个Relation结构体
     Relation结构体 = 目标Attribute Name + Attribute Type(INT, FLOAT, STRING) + sign + key
     例如 where salary > 3.1 中：
     Attribute Name = "salary"
     sign = GREATER
     key.type = FLOAT
     key.FLAOT_VALUE = 3.1
*/
#pragma once

#include "Table.h"

// Sign表示Where条件中的符号
typedef enum {
    NOT_EQUAL,
    EQUAL,
    LESS,
    LESS_OR_EQUAL,
    GREATER_OR_EQUAL,
    GREATER
} Sign_;

class Relation {
public:
    std::string attributeName;
    Sign_ sign;
    key_ key;
};

