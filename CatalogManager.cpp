#include "CatalogManager.h"

void CatalogManager::createTable(std::string table_name, Attribute attribute, int primary, Index index) {
    if (havetable(table_name)) throw table_name_conflict();
    attribute.unique[primary] = true;   //确保主键为unique
    std::string str_tmp="0000 ";    //table信息的大小
    //添加name
    str_tmp += table_name;
    //添加attribute的数量
    str_tmp=str_tmp + " " + encodeNum(attribute.num, 2);
    //添加每个attribute的信息，顺序为类型，名字，是否为唯一
    for(int i=0;i<attribute.num;i++)
        str_tmp=str_tmp+" "+encodeNum(attribute.type[i], 3)+" "+attribute.name[i]+" "+(attribute.unique[i]==true?"1":"0");
    //添加主键信息
    str_tmp=str_tmp+" "+encodeNum(primary, 2);
    //添加index的数量, ;用来做标记index的开始
    str_tmp=str_tmp+" ;"+encodeNum(index.indexNumber, 2);
    //添加index的信息，顺序为相对位置和名字
    for(int i=0;i<index.indexNumber;i++)
        str_tmp=str_tmp+" "+encodeNum(index.location[i], 2)+" "+index.indexname[i];
    //换行后在结尾接上一个#，每个块以#结尾
    str_tmp=str_tmp+"\n"+"#";
    //更改每条信息的长度的记录
    std::string str_len=encodeNum((int)str_tmp.length()-1, 4);
    str_tmp=str_len+str_tmp.substr(4,str_tmp.length()-4);

    int block_number = getBlockNumber(CATALOG_FILEPATH);
    if (block_number == 0) block_number = 1;
    for (int block_id = 0; block_id < block_number; block_id++) {
        char* buffer = buffer_manager.getPage(CATALOG_FILEPATH, block_id);
        int page_id = buffer_manager.getPageId(CATALOG_FILEPATH, block_id);
        int length = 0;
        while (length < PAGESIZE && buffer[length] != '\0' && buffer[length] != '#') length++;
        if (length + str_tmp.size() < PAGESIZE) {
            //页面有足够空间，开始写入
            if (buffer[length]=='#') buffer[length]='\0';
            else if (length > 0 && buffer[length-1]=='#') buffer[length-1]='\0';    //清除分隔符
            //写入字符串
            strcat(buffer, str_tmp.c_str());
            buffer_manager.setDirty(page_id); //脏页
            return;
        }
    }
    //所有块均满，在新块插入
    char* buffer = buffer_manager.getPage(CATALOG_FILEPATH, block_number);
    int page_id = buffer_manager.getPageId(CATALOG_FILEPATH, block_number);
    strcat(buffer , str_tmp.c_str());
    buffer_manager.setDirty(page_id);
}

bool CatalogManager::havetable(std::string table_name) {
    int block_number = getBlockNumber(CATALOG_FILEPATH);
    if (block_number == 0) block_number = 1;
    int start = 0, end = 0; //定义起止
    for (int block_id = 0; block_id < block_number; block_id++) {
        char* buffer = buffer_manager.getPage(CATALOG_FILEPATH, block_id);
        do{
            if (*buffer == '#') break;   //没有table记录，跳过
            if (table_name == decodeTableName(buffer, start, end)) {
                return true; //得到名为name的表名
            } else {    //计算下一个表的起始位置
                std::string len_s(buffer + start, 4); //截取一个int长
                start += atoi(len_s.c_str());   //加表记录的全长
                if (start == 0) break;  //空文档没有记录
            }
        } while(buffer[start] != '#');  //'#'是结尾标记
    }
    return false;
}

//dropTable()：删除表
//输入：表名
//异常：表不存在，抛出table_not_exist
void CatalogManager::dropTable(std::string table_name) {
    if (havetable(table_name)) throw table_not_exist(); //表不存在
    //定位table的块号和偏移量
    int block_id, offset;
    getTableBlock(table_name, block_id, offset);
    //获得buffer
    char* buffer = buffer_manager.getPage(CATALOG_FILEPATH, block_id);
    int page_id = buffer_manager.getPageId(CATALOG_FILEPATH, block_id);
    //获得buffer的size
    std::string len_s(buffer + offset, 4); //截取一个int长
    int buffer_size = atoi(len_s.c_str());
    //拷贝这个table后面的字符串，覆盖这个table
    int i = offset + buffer_size;
    for (; buffer[i] != '#'; i++)
        buffer[offset++] = buffer[i];
    //加入截止符
    buffer[offset] = '#';
    offset++;
    buffer[offset] = '\0';
    //设置脏页
    buffer_manager.setDirty(page_id);
}

bool CatalogManager::haveAttribute(std::string table_name, std::string attribute_name, int &attribute_num) {
    Attribute attr = getAttribute(table_name);
    for (attribute_num = 0; attribute_num < attr.num; attribute_num++)
        if (attr.name[attribute_num] == attribute_name) return true;
    return false;
}

Attribute CatalogManager::getAttribute(std::string table_name) {
    if (havetable(table_name)) throw table_not_exist(); //表不存在
    Attribute attr;
    //定位table的块号和偏移量
    int block_id, offset;
    getTableBlock(table_name, block_id, offset);
    //获得buffer
    char* buffer = buffer_manager.getPage(CATALOG_FILEPATH, block_id);
    int page_id = buffer_manager.getPageId(CATALOG_FILEPATH, block_id);
    //获得tablename的最后一位
    int nameoffset;
    decodeTableName(buffer, offset, nameoffset);
    //定位attribute开始位置
    offset += nameoffset + 1;
    //获取attribute数量
    attr.num = atoi(std::string(buffer + offset, 2).c_str());
    offset += 3;
    //依次读取attribute
    for (int i = 0; i < attr.num; i++) {
        if (buffer[offset] == '-') {
            attr.type[i] = INT;
            offset += 5;
            for (;buffer[offset] != ' '; offset++)
                attr.name[i] += buffer[offset];
            offset++;
            if (buffer[offset] == '1') attr.unique[i] = true;
            else attr.unique[i] = false;
        }
        else if (atoi(std::string(buffer + offset, 3).c_str()) == 0) {
            attr.type[i] = FLOAT;
            offset += 4;
            for (;buffer[offset] != ' '; offset++)
                attr.name[i] += buffer[offset];
            offset++;
            if (buffer[offset] == '1') attr.unique[i] = true;
            else attr.unique[i] = false;
        }
        else {
            attr.type[i] = atoi(std::string(buffer + offset, 3).c_str());
            offset += 4;
            for (;buffer[offset] != ' '; offset++)
                attr.name[i] += buffer[offset];
            offset++;
            if (buffer[offset] == '1') attr.unique[i] = true;
            else attr.unique[i] = false;
        }
        offset += 2;
    }
    //读取primary key
    if (buffer[offset] == '-') attr.primary_key = -1;
    else attr.primary_key = atoi(std::string(buffer + offset, 2).c_str());
    //读取index
    Index index = getIndex(table_name);
    for (int i=0; i<32; i++) attr.index[i] = false;
    for (int i=0; i<index.indexNumber; i++) attr.index[index.location[i]] = true;

    return attr;
}

void CatalogManager::addIndex(std::string table_name, std::string attribute_name, std::string index_name) {
    //获取属性
    Attribute attr = getAttribute(table_name);
    bool haveAttrName = false;
    for (int i = 0; i < attr.num; i++)
        if (attr.name[i] == attribute_name) haveAttrName = true;
    if (!haveAttrName) throw attribute_not_exist(); //属性不存在
    //获取index
    Index index = getIndex(table_name);
    if (index.indexNumber == 10) throw index_full();    //index已满
    for (int i = 0; i < attr.num; i++) {
        if (attr.name[i] == attribute_name && attr.index[i] == true)
            throw index_exist_conflict();       //index已存在
    }
    for (int i = 0; i < index.indexNumber; i++) {
        if (index.indexname[i] == index_name)
            throw index_name_conflict();    //index名冲突
    }
    //异常检查完毕
    for (int i = 0; i < attr.num; i++) {
        if (attr.name[i] == attribute_name) {
            index.location[index.indexNumber] = i;
            index.indexname[index.indexNumber] = index_name;
            index.indexNumber++;
            break;
        }
    }
    //重建表，将index刷入
    dropTable(table_name);
    createTable(table_name, attr, attr.primary_key, index);
}

void CatalogManager::dropIndex(std::string table_name, std::string index_name){
    //获取属性
    Attribute attr = getAttribute(table_name);
    //获取index
    Index index = getIndex(table_name);
    //搜索这个index
    for (int i = 0; i < index.indexNumber; i++) {
        if (index.indexname[i] == index_name) {
            //找到index
            index.indexname[i] = index.indexname[index.indexNumber - 1];
            index.location[i] = index.location[index.indexNumber - 1];
            index.indexNumber--;
            dropTable(table_name);
            createTable(table_name, attr, attr.primary_key, index);
            return;
        }
    }
    throw index_not_exist();    //索引不存在
}

std::string CatalogManager::findAttributeThroughIndex(std::string table_name, std::string index_name) {
    //获取属性
    Attribute attr = getAttribute(table_name);
    //获取index
    Index index = getIndex(table_name);
    //搜索这个index
    for (int i = 0; i < index.indexNumber; i++) {
        if (index.indexname[i] == index_name) {
            //找到index
            return attr.name[index.location[i]];
        }
    }
    throw index_not_exist();    //索引不存在
}

void CatalogManager::showCatalog(std::string table_name) {
    //获取属性
    Attribute attribute = getAttribute(table_name);
    //获取index
    Index index = getIndex(table_name);
	//输出tablename
	std::cout << "TableName: " << table_name << std::endl;
	//输出Attribute
	size_t max_length = 0;
	for (int i = 0; i < attribute.num; i++) {	//先扫描attribute获取name最大长度
		max_length = max_length > attribute.name[i].length() ? max_length : attribute.name[i].length();
	}
	max_length = max_length > 4 ? max_length : 4;
	std::cout << "Attribute Info:" << std::endl;
	std::cout.setf(std::ios::left);
	for (size_t i = 0; i < 6 + max_length + 1 + 6 + 8 + 8 + 4; i++)std::cout << "-";
	std::cout << std::endl;
	std::cout.width(6);
	std::cout << "Num";
	std::cout << "|";
	std::cout.width(max_length + 1);
	std::cout << "Name";
	std::cout << "|";
	std::cout.width(6);
	std::cout << "Type";
	std::cout << "|";
	std::cout.width(8);
	std::cout << "Unique";
	std::cout << "|";
	std::cout.width(8);
	std::cout << "Primary";
	std::cout << std::endl;

	std::cout.fill('-');
	std::cout.width(6);
	std::cout << "-";
	std::cout << "+";
	std::cout.width(max_length + 1);
	std::cout << "-";
	std::cout << "+";
	std::cout.width(6);
	std::cout << "-";
	std::cout << "+";
	std::cout.width(8);
	std::cout << "-";
	std::cout << "+";
	std::cout.width(8);
	std::cout << "-";
	std::cout << std::endl;
	std::cout.fill(' ');
	for (int i = 0; i < attribute.num; i++) {
		std::cout.width(6);
		std::cout << i;
		std::cout << "|";
		std::cout.width(max_length + 1);
		std::cout << attribute.name[i];
		std::cout << "|";
		std::cout.width(6);
		if (attribute.type[i] == INT) std::cout << "INT";
		else if (attribute.type[i] == FLOAT) std::cout << "FLOAT";
		else std::cout << "CHAR";
		std::cout << "|";
		std::cout.width(8);
		if (attribute.unique[i] == true) std::cout << "Unique";
		else std::cout << " ";
		std::cout << "|";
		std::cout.width(8);
		if (i == attribute.primary_key)std::cout << "Primary";
		else std::cout << "";
		std::cout << std::endl;
	}
	for (size_t i = 0; i < 6 + max_length + 1 + 6 + 8 + 8 + 4; i++)std::cout << "-";
	std::cout << std::endl;
	//打印index
	size_t attr_max_length = max_length > 15 ? max_length:15;
	max_length = 0;
	for (int i = 0; i < index.indexNumber; i++) {	//先扫描index获取name最大长度
		max_length = max_length > index.indexname[i].length() ? max_length : index.indexname[i].length();
	}
	max_length = max_length > 12 ? max_length : 12;
	std::cout << "Index Info:" << std::endl;
	for (size_t i = 0; i < max_length + attr_max_length + 2; i++) std::cout << "-";
	std::cout << std::endl;
	std::cout.setf(std::ios::left);
	std::cout.width(max_length + 1);
	std::cout << "Index Name";
	std::cout << "|";
	std::cout << "Attribute Name";
	std::cout << std::endl;
	std::cout.fill('-');
	std::cout.width(max_length + 1);
	std::cout << "-";
	std::cout << "+";
	std::cout.width(attr_max_length);
	std::cout << "-";
	std::cout << std::endl;
	std::cout.fill(' ');
	for (int i = 0; i < index.indexNumber; i++) {
		std::cout.width(max_length + 1);
		std::cout << index.indexname[i];
		std::cout << "|";
		std::cout << attribute.name[index.location[i]];
		std::cout << std::endl;
	}
	for (size_t i = 0; i < max_length + attr_max_length + 2; i++) std::cout << "-";
	std::cout << std::endl;
}


// block: (4 bytes table长度 + table) * n + '#'
void CatalogManager::getTableBlock(std::string name, int& block_id, int& offset) {
    int block_number = getBlockNumber(CATALOG_FILEPATH);
    if (block_number == 0) block_number = 1;
    int start = 0, end = 0; //定义起止
    for (block_id = 0; block_id < block_number; block_id++) {
        char* buffer = buffer_manager.getPage(CATALOG_FILEPATH, block_id);
        do{
            if (*buffer == '#') break;   //没有table记录，跳过
            if (name == decodeTableName(buffer, start, end)) {
                offset = start;
                return; //得到名为name的表名
            } else {    //计算下一个表的起始位置
                std::string len_s(buffer + start);
                len_s = len_s.substr(0, 4); //截取一个int长
                start += atoi(len_s.c_str());   //加表记录的全长
                if (start == 0) break;  //空文档没有记录
            }
        } while(buffer[start] != '#');  //'#'是结尾标记
    }
}

int CatalogManager::getBlockNumber(std::string filepath) {
    int block_id = 0;
    while (*buffer_manager.getPage(filepath, block_id) != '\0')
        block_id++;
    return block_id;
}

std::string CatalogManager::decodeTableName(char* buffer, int start, int& end) {
    if (buffer[0]=='\0') return ""; //buffer为空
    int offset = 0;
    while (buffer[start + 5 + offset] != ' ') offset++;   //检测分隔符' '
    end = start + 5 + offset;
    return std::string(buffer + start + 5).substr(0, offset);   //截取表名
}

Index CatalogManager::getIndex(std::string table_name) {
    Index index;
    int block_id, block_offset;
    getTableBlock(table_name, block_id, block_offset);    //得到该表的位置和对应的块
    char* buffer = buffer_manager.getPage(CATALOG_FILEPATH, block_id);
    //从block_offset开始，寻找index的位置
    while (buffer[block_offset] != ';') block_offset++;
    block_offset += 1;    //对齐至index起始位置
    std::string s_index(buffer + block_offset);
    //得到索引的数量(宽度为2)
    index.indexNumber = atoi(s_index.substr(0,2).c_str());
    //得到索引的所有信息
    for(int i = 0, offset = 0; i<index.indexNumber; i++){
        offset+=3;
        index.location[i] = atoi(s_index.substr(0,2).c_str());
        offset+=3;
        while(s_index[offset]!=' '&&s_index[offset]!='#'&&s_index[offset]!='\n'){
            index.indexname[i] += s_index[offset++];
        }
        offset-=2;
    }
    return index;
}

std::string CatalogManager::encodeNum(int num, short width) {
    std::string s = "";
    int div = 1;
    if (num < 0) {
        s = s + "-";
        num = -num;
    }
    for (int i = 0; i < width - 1; i++) div *= 10;
    for (int i = 0; i < width; i++, div/=10) {
        char c = num / div % 10 + '0';
        s = s + c;
    }
    return s;
}