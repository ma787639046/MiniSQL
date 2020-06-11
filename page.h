#pragma once

#include <iostream>
#include <vector>
#include <string>
#include "const.h"

class Page {
public:
    Page() { clear(); };
    void clear();   //清空页信息
    void setFileName(std::string file_name);
    std::string getFileName();
    void setBlockId(int block_id);
    int getBlockId();
    void pin();
    void unpin();
    int getPinNum();
    void setDirty(bool dirty);
    bool getDirty();
    void setFirstLoaded(bool ref);
    bool getFirstLoaded();
    void setValid(bool valid);
    bool getValid();
    char* getPagePointer();
private:
    char page[PAGESIZE];//每一页都是一个大小为PAGESIZE字节的数组
    std::string filename;//页所对应的文件名
    int blockID;//页在所在文件中的块号(磁盘中通常叫块)
    int pinNum;//记录被钉住的次数。被钉住的意思就是不可以被替换
    bool isDirty;//dirty记录页是否被修改
    bool clockReference;//ref变量用于时钟替换策略
    bool valid;//avaliable标示页是否可以被使用(即将磁盘块load进该页)
};

inline void Page::setFileName(std::string file_name) {
    filename = file_name;
}

inline std::string Page::getFileName() {
    return filename;
}

inline void Page::setBlockId(int block_id) {
    blockID = block_id;
}

inline int Page::getBlockId() {
    return blockID;
}

inline void Page::pin() {
    pinNum++;
}

inline void Page::unpin() {
    pinNum--;
}

inline int Page::getPinNum() {
    return pinNum;
}

inline void Page::setDirty(bool dirty) {
    isDirty = dirty;
}

inline bool Page::getDirty() {
    return isDirty;
}

inline void Page::setFirstLoaded(bool ref) {
    clockReference = ref;
}

inline bool Page::getFirstLoaded() {
    return clockReference;
}

inline void Page::setValid(bool valid) {
    this->valid = valid;
}

inline bool Page::getValid() {
    return valid;
}

inline char* Page::getPagePointer() {
    return page;
}