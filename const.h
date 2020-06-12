#pragma once

// 每一页的大小为4KB
#define PAGESIZE 4096
// 最大页数为100
#define MAXBUFFERSIZE 100

/* catalog文件路径 */
#define CATALOG_FILEPATH "./data/catalog/catalog.db"

/* record文件夹路径。每张表有一个record，record的filepath = RECORD_PATH + table_name + ".db" */
#define RECORD_PATH "./data/record/"

/* index文件夹路径。每张表有一个index文件，index的filepath = INDEX_PATH + table_name + ".db" */
#define INDEX_PATH "./data/index/"