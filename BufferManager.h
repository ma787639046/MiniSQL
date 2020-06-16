#pragma once

#include "page.h"

class BufferManager {
public: 
    BufferManager() { newBuffer(MAXBUFFERSIZE);};
    BufferManager(int page_num) { newBuffer(page_num);};
    ~BufferManager();
    // getPage()：通过页号得到PAGESIZE大小的page的地址
    // 输入：文件名，块号
    // 返回：PAGESIZE大小的page首地址
    char* getPage(std::string file_name , int blockID);
    // setDirty()：标记pageID所对应的页已经被修改
    // 输入：page_id
    void setDirty(int pageID);
    // pinPage()：pin某个page一次
    // 输入：page_id
    void pinPage(int pageID);
    // unpinPage()：unpin某个page一次
    // 输入：page_id
    // 返回：如果对应页不再被pin，则返回-1
    int unpinPage(int pageID);
    // swapOutPage()：将页写入对应文件的对应块。
    // 输入：页号，文件名，块号
    void swapOutPage(int pageID , std::string file_name , int blockID);
    // getPageId()：通过blockID获取pageID
    // 输入：文件名，块号
    // 返回：页号
    int getPageId(std::string file_name , int blockID);
private:
    // 页数组，作为buffer pool
    Page* BufferPool;
    // 总页数
    int PageNum;
    // 页替换指针标记
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