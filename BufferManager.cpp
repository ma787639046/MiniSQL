//
//  BufferManager.cpp
//  Created by MGY on 2020/06/02
//

#include <BufferManager.h>
#include <const.h>

// Page类
Page::Page() {
    memset(buffer, 0, PAGESIZE);
    block_id = -1;
    pinNum = 0;
    dirty = false;
    valid = false;
}

Page::~Page() {}

inline char* Page::getPagePointer() {
    return this->buffer;
}

inline void Page::setBlockID(int ID) {
    this->block_id = ID;
}

inline int Page::getBlockID() {
    return this->block_id;
}

inline void Page::pin()
{
    this->pinNum++;
}

inline void Page::unpin()
{
    this->pinNum--;
}

inline int Page::getPinNum()
{
    return this->pinNum;
}

inline bool Page::getDirty()
{
    return this->dirty;
}

inline void Page::setDirty(bool dirty)
{
    this->dirty = dirty;
}

inline bool Page::getValid()
{
    return this->valid;
}

inline void Page::setValid(bool valid)
{
    this->valid = valid;
}

BufferManager::BufferManager(std::string filename, size_t buffer_size)
{
    Buffer.reserve(buffer_size); //保留buffer_size个Page的空间，以提高malloc速度
    this->filename = filename;
    this->buffer_size = buffer_size;
}

BufferManager::~BufferManager()
{
    for (int i = 0; i < (int)Buffer.size(); i++) {
        if (Buffer[i].getValid() == true && Buffer[i].getDirty() == true) {
            swapOutPage(i); // valid且dirty，swap out这个Page
        }
    }
    Buffer.clear();
}

std::string BufferManager::getFilename()
{
    return this->filename;
}

int BufferManager::getBlockID(int page_id)
{
    return Buffer[page_id].getBlockID();
}

int BufferManager::getPageID(int block_id)
{
    //从头线性扫描所有Page，获得对应的PageID。没有则返回-1
    for (int i = 0; i < (int)Buffer.size(); i++) {
        if (this->filename == filename && Buffer[i].getBlockID() == block_id) {
            return i;
        }
    }
    return -1;
}

int BufferManager::loadBlock(int block_id)
{
    int PageID = -1;
    if (Buffer.size() < this->buffer_size) {
        Page* page = new Page();
        Buffer.push_back(*page);    //分配页
        PageID = Buffer.size() - 1;
    }
    else {
        //Buffer已满，先寻找valid==false的页
        for (int i = 0; i < (int)Buffer.size(); i++) {
            if (Buffer[i].getValid() == false) PageID = i;
        }
        //找不到unvalid的Page，则swap out一个Page。（这里有待加入一个页替换策略）
        if (PageID == -1) {
            swapOutPage(0);
            PageID = 0;
        }
    }
    FILE* file = fopen((this->filename).c_str(), "r");
    if (file == nullptr) {  //读不到file，则创建空文件。理论上，文件由Record Manager、Catalog Manager等模块创建、删除。文件是否存在应由上游模块检查。 
        file = fopen((this->filename).c_str(), "w");
        fwrite("", 1, 0, file);
        fclose(file);
        file = fopen((this->filename).c_str(), "r");
    }
    fseek(file, block_id * PAGESIZE, 0);    //定位文件指针
    fread(Buffer[PageID].getPagePointer(), PAGESIZE, 1, file);  //读取PAGESIZE bytes至Page中
    fclose(file);
    Buffer[PageID].setBlockID(block_id);
    Buffer[PageID].setDirty(false);
    Buffer[PageID].setValid(true);
    return PageID;
}

void BufferManager::swapOutPage(int page_id)
{
    if (getDirty(page_id) == true) {
        //写入block
        FILE* file = fopen((this->filename).c_str(), "r+");
        fseek(file, Buffer[page_id].getBlockID() * PAGESIZE, 0);    //定位文件指针
        fwrite(getPagePointer(page_id), PAGESIZE, 1, file); //写入
        fclose(file);
    }
    setValid(page_id, false);
}

char* BufferManager::getPagePointer(int page_id)
{
    return Buffer[page_id].getPagePointer();
}

void BufferManager::pinPage(int page_id)
{
    Buffer[page_id].pin();
}

void BufferManager::unpinPage(int page_id)
{
    Buffer[page_id].unpin();
}

bool BufferManager::getPin(int page_id)
{
    return Buffer[page_id].getPinNum() > 0;
}

void BufferManager::setDirty(int page_id, bool status)
{
    Buffer[page_id].setDirty(status);
}

bool BufferManager::getDirty(int page_id)
{
    return Buffer[page_id].getDirty();
}

void BufferManager::setValid(int page_id, bool status)
{
    Buffer[page_id].setValid(status);
}

bool BufferManager::getValid(int page_id)
{
    return Buffer[page_id].getValid();
}

int BufferManager::getBlockNum()
{
    struct _stat info;
    _stat(this->filename.c_str(), &info);
    int size = info.st_size;
    return size/PAGESIZE;
}

//测试用main

//int main() {
//    BufferManager buffer("Buffer.db");
//    int page_id = buffer.loadBlock(buffer.getFilename(), 0);
//    //写入对象的测试
//    /*std::string s = "123456";
//    char* page = buffer.getPagePointer(page_id);
//    memcpy_s(page, PAGESIZE, &s, sizeof(s));
//    buffer.setDirty(page_id, true);*/
//    //读取对象的测试
//    /*char* page = buffer.getPagePointer(page_id);
//    std::string s = *(std::string*)page;
//    std::cout << s << std::endl;*/
//    return 0;
//}
