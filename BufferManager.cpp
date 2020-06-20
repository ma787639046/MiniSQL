#include "BufferManager.h"

BufferManager::~BufferManager() {
    for (int i = 0; i < PageNum; i++)
        if (BufferPool[i].getDirty())
            swapOutPage(i , BufferPool[i].getFileName(), BufferPool[i].getBlockId());
}

char* BufferManager::getPage(std::string file_name , int blockID) {
    int pageID = getPageId(file_name , blockID);
    if (pageID == -1) {
        pageID = getEmptyPageId();
        loadDiskBlock(pageID , file_name , blockID);
    }
    BufferPool[pageID].setFirstLoaded(true);
    return BufferPool[pageID].getPagePointer();
}

void BufferManager::setDirty(int pageID) {
    BufferPool[pageID].setDirty(true);
}

void BufferManager::pinPage(int pageID) {
    BufferPool[pageID].pin();
}

int BufferManager::unpinPage(int pageID) {
    int pin_count = BufferPool[pageID].getPinNum();
    if (pin_count <= 0) return -1;
    BufferPool[pageID].unpin();
    return 0;
}

int BufferManager::loadDiskBlock(int pageID , std::string file_name , int blockID) {
    FILE* file = fopen(file_name.c_str() , "rb");
    if (file == nullptr) return -1; //找不到文件
    fseek(file , PAGESIZE * blockID , SEEK_SET);
    char* buffer = BufferPool[pageID].getPagePointer();
    memset(buffer, 0, PAGESIZE);
    fread(buffer , PAGESIZE , 1 , file);
    fclose(file);
    // 初始化页
    BufferPool[pageID].setFileName(file_name);
    BufferPool[pageID].setBlockId(blockID);
    BufferPool[pageID].setDirty(false);
    BufferPool[pageID].setFirstLoaded(true);    // 第一次不会被替换
    BufferPool[pageID].setValid(true);
    return 0;
}

void BufferManager::swapOutPage(int pageID , std::string file_name , int blockID) {
    FILE* file = fopen(file_name.c_str() , "rb+");
    if (file == nullptr) return; //找不到文件
    fseek(file , PAGESIZE * blockID , SEEK_SET);
    char* buffer = BufferPool[pageID].getPagePointer();
    fwrite(buffer , PAGESIZE , 1 , file);
    BufferPool[pageID].setValid(false);
    fclose(file);
}

int BufferManager::getPageId(std::string file_name , int blockID) {
    for (int i = 0;i < PageNum;i++) {
        if (BufferPool[i].getFileName() == file_name && BufferPool[i].getBlockId() == blockID)
            return i;
    }
    return -1;
}

int BufferManager::getEmptyPageId() {
    int i = 0;
    while (i < PageNum) {
        if (BufferPool[i].getValid() == false) return i;
        i++;
    }
    while (true) {
        if (BufferPool[clockPos].getFirstLoaded() == true) {
            BufferPool[clockPos].setFirstLoaded(false);
            clockPos = (clockPos + 1) % PageNum;
        }
        else if (BufferPool[clockPos].getPinNum() == 0) {   //unpinned
            if (BufferPool[clockPos].getDirty() == true) //需要写回磁盘
                swapOutPage(clockPos , BufferPool[clockPos].getFileName(), BufferPool[clockPos].getBlockId());
            return clockPos;  // 返回页号
        }
    }
}