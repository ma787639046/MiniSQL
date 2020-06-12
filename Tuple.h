#pragma once

#include <iostream>
#include <vector>

typedef enum{
    INT = -1,
    FLOAT = 0
    //STRING > 0 (string的长度+1)
}keyType;

//type的类型：INT, FLOAT, STRING对应char(n)中的n
struct key_{
    int type;
    int INT_VALUE;
    float FLOAT_VALUE;
    std::string STRING_VALUE;
};

class Tuple{
private:
    std::vector<key_> keys;  //存储元组里的每个数据的信息
    bool Available;
public:
    Tuple() : Available(true) {};
    Tuple(const Tuple &tuple) {
        Available = tuple.Available;
        for(size_t i=0;i<tuple.keys.size();i++)
            this->keys.push_back(tuple.keys[i]);
    };
    void addKey(key_ key);  //新增元组
    std::vector<key_> getKeys();  //返回数据
    int size();  //返回元组的数据数量
    bool isAvailable();
    void setAvailable(bool status);
    void showTuple();  //显示元组中的所有数据
};

inline void Tuple::addKey(key_ key) {
    keys.push_back(key);
}

inline std::vector<key_> Tuple::getKeys() {
    return keys;
}

inline int Tuple::size() {
    return (int)keys.size();
}

inline bool Tuple::isAvailable() {
    return Available;
}

inline void Tuple::setAvailable(bool status) {
    Available = status;
}