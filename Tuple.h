#pragma once

#include <iostream>
#include <vector>

typedef enum{
    INT = -1,
    FLOAT = 0
    //STRING > 0，对应char(n)中的n
}keyType;

// key = type + VALUE(INT, FLOAT, >0(STRING))
// type的类型：INT, FLOAT, STRING对应char(n)中的n
struct key_{
    int type;
    int INT_VALUE;
    float FLOAT_VALUE;
    std::string STRING_VALUE;
};

// Tuple = {key1, key2, key3, key4, ……}
class Tuple{
private:
    std::vector<key_> keys;
public:
    Tuple() {};
    Tuple(const Tuple &tuple) {
        for(size_t i=0;i<tuple.keys.size();i++)
            this->keys.push_back(tuple.keys[i]);
    };
    void addKey(key_ key);
    std::vector<key_> getKeys();
    int size();
    void showTuple();
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