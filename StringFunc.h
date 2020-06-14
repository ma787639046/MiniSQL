// StringFunc.h : This file contains some functions used to operate type std::String
//
#pragma once

#include <iostream>
#include <string>
#include <vector>


template <class T>
T string2num(std::string &s)
{
    std::istringstream iss(s);
    T ret;
    iss >> ret;
    return ret;
}

//split()：以delimiters分割字符串，将结果存至tokens
//输入：待分割的string& s，分割结果vector<string>& tokens，分隔符string& delimiters
//返回：无
void split(const std::string& s, std::vector<std::string>& tokens, const std::string& delimiters = " ")
{
    std::string::size_type lastPos = s.find_first_not_of(delimiters, 0);
    std::string::size_type pos = s.find_first_of(delimiters, lastPos);
    while (std::string::npos != pos || std::string::npos != lastPos) {
        tokens.push_back(s.substr(lastPos, pos - lastPos));
        lastPos = s.find_first_not_of(delimiters, pos);
        pos = s.find_first_of(delimiters, lastPos);
    }
}

//trim()：去除首位两端的特定字符
//输入：字符串引用，要去除的首位字符spliter
//返回：无
void trim(std::string& s, std::string spliter)
{
    if (s.empty()) return;
    s.erase(0, s.find_first_not_of(spliter));
    s.erase(s.find_last_not_of(spliter) + 1);
}


//for each char in data, p[offset+i] <- data[i]
template <typename T>
void string_duplicate(char *p, int &offset, T data)
{
    std::stringstream stream;
    stream << data;
    std::string s1 = stream.str();
    for(int i=0;i<s1.length();i++,offset++)
    {
        p[offset] = s1[i];
    }
}
