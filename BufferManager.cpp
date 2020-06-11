#include "BufferManager.h"

// 析构函数:将缓冲池里的所有页写回磁盘。
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
    if (pin_count <= 0) 
        return -1;
    else
        BufferPool[pageID].unpin();
    return 0;
}

// 核心函数之一。内存和磁盘交互的接口。
int BufferManager::loadDiskBlock(int pageID , std::string file_name , int blockID) {
    // 初始化一个页
    // BufferPool[pageID].clear();
    // 打开磁盘文件
    FILE* file = fopen(file_name.c_str() , "r");
    // 打开失败返回-1
    if (file == NULL)
        return -1;
    // 将文件指针定位到对应位置
    fseek(file , PAGESIZE * blockID , SEEK_SET);
    // 获取页的句柄
    char* buffer = BufferPool[pageID].getPagePointer();
    // 读取对应磁盘块到内存页
    fread(buffer , PAGESIZE , 1 , file);
    // 关闭文件
    fclose(file);
    // 对新载入的页进行相应设置
    BufferPool[pageID].setFileName(file_name);
    BufferPool[pageID].setBlockId(blockID);
    BufferPool[pageID].setDirty(false);
    BufferPool[pageID].setFirstLoaded(true);    // 第一次不会被替换
    BufferPool[pageID].setValid(false);
    return 0;
}

// 核心函数之一。内存和磁盘交互的接口。
void BufferManager::swapOutPage(int pageID , std::string file_name , int blockID) {
    // 打开文件
    FILE* file = fopen(file_name.c_str() , "r+");
    // 将文件指针定位到对应位置
    fseek(file , PAGESIZE * blockID , SEEK_SET);
    // 获取页的句柄
    char* buffer = BufferPool[pageID].getPagePointer();
    // 将内存页的内容写入磁盘块
    fwrite(buffer , PAGESIZE , 1 , file);
    // 关闭文件
    fclose(file);
}

// 简单遍历获取页号
int BufferManager::getPageId(std::string file_name , int blockID) {
    for (int i = 0;i < PageNum;i++) {
        if (BufferPool[i].getFileName() == file_name && BufferPool[i].getBlockId() == blockID)
            return i;
    }
    return -1;
}

// 寻找一个闲置的页
int BufferManager::getEmptyPageId() {
    // 先简单的遍历一遍，如果有闲置的直接返回其页号
    for (int i = 0;i < PageNum;i++) {
        if (BufferPool[i].getValid() == true)
            return i;
    }
    while (1) {
        // 如果页的ref为true，将其设为false
        if (BufferPool[clockPos].getFirstLoaded() == true) BufferPool[clockPos].setFirstLoaded(false);      
        // 否则，如果页没有被锁住，那么这一页就会被删除
        else if (BufferPool[clockPos].getPinNum() == 0) {
            // 如果这一页被改动过，需要将其写回磁盘，然后删除
            if (BufferPool[clockPos].getDirty() == true) {
                std::string file_name = BufferPool[clockPos].getFileName();
                int blockID = BufferPool[clockPos].getBlockId();
                swapOutPage(clockPos , file_name , blockID);
            }
            // 删除页
            BufferPool[clockPos].clear();
            // 返回页号
            return clockPos;
        }
        // 时钟指针顺时针转动
        clockPos = (clockPos + 1) % PageNum;
    }
}