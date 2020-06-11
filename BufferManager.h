#pragma once

#include "page.h"

class BufferManager {
public: 
    // 构造函数
    BufferManager() { newBuffer(MAXBUFFERSIZE);};
    BufferManager(int page_num) { newBuffer(page_num);};
    // 析构函数
    ~BufferManager();
    // 通过页号得到页的地址
    char* getPage(std::string file_name , int blockID);
    // 标记pageID所对应的页已经被修改
    void setDirty(int pageID);
    // 钉住一个页
    void pinPage(int pageID);
    // 解除一个页的钉住状态(需要注意的是一个页可能被多次钉住，该函数只能解除一次)
    // 如果对应页的pin为0，则返回-1
    int unpinPage(int pageID);
    // 将对应内存页写入对应文件的对应块。
    void swapOutPage(int pageID , std::string file_name , int blockID);
    // 获取对应文件的对应块在内存中的页号，没有找到返回-1
    int getPageId(std::string file_name , int blockID);
private:
    // 页数组，作为buffer pool
    Page* BufferPool;
    // 总页数
    int PageNum;
    int clockPos;
    // 初始化函数：设置page_num个页的buffer
    void newBuffer(int page_num);
    // 获取一个闲置的页的页号
    int getEmptyPageId();
    // 将对应文件的对应块载入对应内存页，对于文件不存在返回-1，否则返回0
    int loadDiskBlock(int pageID , std::string file_name , int blockID);
};


inline void BufferManager::newBuffer(int page_num) {
    BufferPool = new Page[page_num];
    PageNum = page_num;
    clockPos = 0;
}

/*
    Buffer的页替换策略如下：
    定义一个时钟指针clockPos，它将顺时针转动，转动时，如果遇到clockReference == true，就变为false；
    如果遇到clockReference == false，就返回这个页
    当一个页被load后，clockReference = true;
*/