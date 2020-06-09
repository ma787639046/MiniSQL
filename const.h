//
//  const.h
//  Created by MGY on 2020/06/02
//

//  const.h 宏定义const的值
//

//#pragma once

//buffer_manager常量
#define PAGESIZE 8192   // 定义PAGESIZE，代表缓冲区中单个页的大小

#define ENTITY_DIVIDER "##$%"	//利用ENTITY_DIVIDER，在一个page中分割不同的
								//Entity。例如：##$%Data1##$%Data2

//catalog文件路径
#define CATALOG_FILEPATH "./data/catalog/catalog.db"

//record文件夹路径。每张表有一个record，record的filepath = RECORD_PATH + table_name + ".db"
#define RECORD_PATH "./data/record/"