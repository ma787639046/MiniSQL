//
//  BufferManager.h
//  Created by MGY on 2020/06/02
//

// Buffer Manager负责缓冲区的管理，主要功能有：
// 1.	根据需要，读取指定的数据到系统缓冲区或将缓冲区中的数据写出到文件
// 2.	实现缓冲区的替换算法，当缓冲区满时选择合适的页进行替换
// 3.	记录缓冲区中各页的状态，如是否被修改过等
// 4.	提供缓冲区页的pin功能，及锁定缓冲区的页，不允许替换出去

#pragma once

#include <iostream>
#include <const.h>
#include <string>
#include <cstring>
#include <vector>
#include <fstream>
#include <io.h>

#define BUFFERSIZE 128*250	// 定义BUFFERSIZE，128*x 代表 x Mb

/*
    Page类：Page类代表Buffer中的一个页（Page）。一个Page通常对应一个磁盘文件中的块（block）。
            Page类的本质是一个char buffer[PAGESIZE]数组，附带block_id，filename，pinNum，
            dirty，valid等标志信息，及相关的函数。
*/
class Page {
public:
    Page();
    ~Page();
    char* getPagePointer();    //获取Page的首地址
    void setBlockID(int ID);
    int getBlockID();
    /*void setFileName(std::string filename);
    std::string getFileName();*/
    void pin();
    void unpin();
    int getPinNum();
    bool getDirty();
    void setDirty(bool dirty);
    bool getValid();
    void setValid(bool valid);

private:
    char buffer[PAGESIZE];  // 利用一个大小为PAGESIZE的char数组，表示一个页
    // 以下变量记录了这个Page所映射的，在磁盘文件中的block信息
    int block_id;   // 磁盘文件中的块号。通常一个Page <-> 一个磁盘block，在磁盘文件中，块号为block_id，起始位置对应PAGESIZE*block_id。
    // 以下变量记录了Page的状态，包括pin（上锁、信号量）、dirty（是否有修改）、valid（是否可用）
    int pinNum;    // pinNum以信号量的方式，表示Page的锁定状态。每pin一次Page，pin++；当pin==0时，Page可swap out；当pin>0时，Page不可swap out。
    bool dirty; // dirty表示Page是否有修改。当dirty==true时，swap out这个Page时，必须讲此页面写入磁盘对应的block。
    bool valid; // 表示Page是否可用。当Page初始化、或swap out时，valid==false；当Page已load对应的block的数据，valid==true
};

class BufferManager {
public:
    BufferManager(std::string filename, size_t buffer_size);
    ~BufferManager();   // 构造和析构函数
    // 以下函数用于buffer中的Page与磁盘中的block的交互
    std::string getFilename();  //获取Buffer对应的磁盘filename
    int getBlockID(int page_id);    //通过page_id得到block_id，没找到则返回-1
    int getPageID(int block_id);  //通过block_id得到page_id，没找到则返回-1（未找到，说明这个block可能被swap out或未装入buffer）
    int loadBlock(int block_id); //将Block装入内存，返回对应的PageID
    void swapOutPage(int page_id);  //将Page swap out至对应block中，同时设置valid=false

    char* getPagePointer(int page_id);  //通过PageID得到Page的头指针
    void pinPage(int page_id);  //Pin一个Page
    void unpinPage(int page_id);    //unpin一个Page
    bool getPin(int page_id);   //获取Page的pin标志位
    void setDirty(int page_id, bool status); //设置Page的dirty
    bool getDirty(int page_id); //获取Page的dirty标志位
    void setValid(int page_id, bool status); //设置Page的valid
    bool getValid(int page_id);
    int getBlockNum();  //通过文件大小，计算block的数量

private:
    std::vector<Page> Buffer;   // Buffer为一个vector，包含特定数量的Page
    std::string filename;   // Buffer对应的filename
    size_t buffer_size; //Buffer推荐的大小，以页数为单位
};
