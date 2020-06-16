#pragma once

#include <iostream>
#include <vector>
#include <string>
#include "const.h"

class Page {
public:
    Page() {
        filename = "";
        blockID = -1;
        pinNum = 0;
        isDirty = false;
        clockReference = false;
        valid = false;
        memset(page, 0, PAGESIZE);
    };
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
    char page[PAGESIZE];
    std::string filename;
    int blockID;
    int pinNum;
    bool isDirty;
    bool clockReference;
    bool valid;
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