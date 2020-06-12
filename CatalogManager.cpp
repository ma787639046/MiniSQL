//
//	CatalogManager.cpp
//	Created by MGY on 2020/06/04
//

#include "CatalogManager.h"

CatalogManager::CatalogManager()
{
	//没有catalog file，自动建立新的catalog file
	if (!exists_file(CATALOG_FILEPATH)) createCatalogFile();
}

void CatalogManager::createTable(std::string table_name, Attribute attribute, Index index)
{
	//这里判断是否存在表名相同的表。存在则抛出table_name_conflict异常
	if (havetable(table_name)) throw table_name_conflict();
	Catalog new_catalog;	//建立new_catalog对象
	new_catalog.TableName = table_name;
	new_catalog.attribute = attribute;
	new_catalog.index = index;	//给new_catalog对象赋值
	// 保证primary key是unique的
	if (new_catalog.attribute.primary_key != -1) new_catalog.attribute.unique[new_catalog.attribute.primary_key] = true;
	// encode这个catalog
	std::string encodedCatalog = encodeCatalog(new_catalog);
	// 在最后一块block上写入新的table
	int blocknum = getBlockNumber();
	char* pointer = buffer_manager.getPage(CATALOG_FILEPATH, blocknum - 1);
	if (getBlockStringSize(pointer) + encodedCatalog.size() < PAGESIZE) {
		// 空间足够，开始写入
		writeTableString(pointer, encodedCatalog);
		buffer_manager.setDirty(buffer_manager.getPageId(CATALOG_FILEPATH, blocknum - 1));
	}
	else {
		// 空间不足，在新块写入
		pointer = buffer_manager.getPage(CATALOG_FILEPATH, blocknum);
		// 先建立块头
		std::string blockhead = createFormatedBlockString();
		for (size_t i = 0; i < blockhead.size(); i++) pointer[i] = blockhead[i];
		// 开始写入
		writeTableString(pointer, encodedCatalog);
		buffer_manager.setDirty(buffer_manager.getPageId(CATALOG_FILEPATH, blocknum - 1));
	}
}

bool CatalogManager::havetable(std::string table_name)
{
	//从第二个block开始搜索
	for (int i = 1; i < getBlockNumber(); i++) {
		char* pointer = buffer_manager.getPage(CATALOG_FILEPATH, i);	//加载block i
		std::vector<Catalog> exist_catalogs = decodeAllCatalog(pointer);	//得到block中现有的catalog
		for (size_t j = 0; j < exist_catalogs.size(); j++) {
			if (exist_catalogs[j].TableName == table_name) {
				//存在同名表
				return true;
			}
		}
	}
	return false;
}

int CatalogManager::findTableBlock(std::string table_name)
{
	//从第二个block开始搜索
	for (int i = 1; i < getBlockNumber(); i++) {
		char* pointer = buffer_manager.getPage(CATALOG_FILEPATH, i);	//加载block i
		std::vector<Catalog> exist_catalogs = decodeAllCatalog(pointer);	//得到block中现有的catalog
		for (size_t j = 0; j < exist_catalogs.size(); j++) {
			if (exist_catalogs[j].TableName == table_name) {
				//存在同名表
				return i;
			}
		}
	}
	return -1;	//找不到名称为table_name的表
}

void CatalogManager::dropTable(std::string table_name)
{

	int block_id = findTableBlock(table_name);
	if (block_id == -1) throw table_not_exist();	//表不存在，抛出table_not_exist
	// 装入
	char* pointer = buffer_manager.getPage(CATALOG_FILEPATH, block_id);
	std::vector<Catalog> catalog = decodeAllCatalog(pointer);
	// 找到并删除对应的catalog
	for (std::vector<Catalog>::iterator i = catalog.begin(); i != catalog.end(); i++) {
		if ((*i).TableName == table_name) {
			catalog.erase(i);
			break;
		}
	}
	// 将catalog重写写回
	// 先建立块头
	std::string blockhead = createFormatedBlockString();
	for (size_t i = 0; i < blockhead.size(); i++) pointer[i] = blockhead[i];
	// 开始写入
	for (size_t i = 0; i < catalog.size(); i++) {
		std::string encodedstring = encodeCatalog(catalog[i]);
		writeTableString(pointer, encodedstring);
	}
	buffer_manager.setDirty(buffer_manager.getPageId(CATALOG_FILEPATH, block_id));
}

Catalog CatalogManager::getCatalog(std::string table_name)
{

	int block_id = findTableBlock(table_name);
	if (block_id == -1) throw table_not_exist();
	else {
		// 装入
		char* pointer = buffer_manager.getPage(CATALOG_FILEPATH, block_id);
		std::vector<Catalog> catalog = decodeAllCatalog(pointer);
		// 找到并删除对应的catalog
		for (std::vector<Catalog>::iterator i = catalog.begin(); i != catalog.end(); i++) {
			if ((*i).TableName == table_name) {
				return *i;
			}
		}
	}
	throw table_not_exist();	//这一步理论上不会发生
	return Catalog();
}

void CatalogManager::showCatalog(std::string table_name)
{
	Catalog catalog = getCatalog(table_name);
	//输出tablename
	std::cout << "TableName: " << catalog.TableName << std::endl;
	//输出Attribute
	size_t max_length = 0;
	for (int i = 0; i < catalog.attribute.num; i++) {	//先扫描attribute获取name最大长度
		max_length = max_length > catalog.attribute.name[i].length() ? max_length : catalog.attribute.name[i].length();
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
	for (int i = 0; i < catalog.attribute.num; i++) {
		std::cout.width(6);
		std::cout << i;
		std::cout << "|";
		std::cout.width(max_length + 1);
		std::cout << catalog.attribute.name[i];
		std::cout << "|";
		std::cout.width(6);
		if (catalog.attribute.type[i] == INT) std::cout << "INT";
		else if (catalog.attribute.type[i] == FLOAT) std::cout << "FLOAT";
		else std::cout << "CHAR";
		std::cout << "|";
		std::cout.width(8);
		if (catalog.attribute.unique[i] == true) std::cout << "Unique";
		else std::cout << " ";
		std::cout << "|";
		std::cout.width(8);
		if (i == catalog.attribute.primary_key)std::cout << "Primary";
		else std::cout << "";
		std::cout << std::endl;
	}
	for (size_t i = 0; i < 6 + max_length + 1 + 6 + 8 + 8 + 4; i++)std::cout << "-";
	std::cout << std::endl;
	//打印index
	size_t attr_max_length = max_length > 15 ? max_length : 15;
	max_length = 0;
	for (int i = 0; i < catalog.index.indexNumber; i++) {	//先扫描index获取name最大长度
		max_length = max_length > catalog.index.indexname[i].length() ? max_length : catalog.index.indexname[i].length();
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
	for (int i = 0; i < catalog.index.indexNumber; i++) {
		std::cout.width(max_length + 1);
		std::cout << catalog.index.indexname[i];
		std::cout << "|";
		std::cout << catalog.attribute.name[catalog.index.location[i]];
		std::cout << std::endl;
	}
	for (size_t i = 0; i < max_length + attr_max_length + 2; i++) std::cout << "-";
	std::cout << std::endl;
}

bool CatalogManager::haveAttribute(std::string table_name, std::string attribute_name, int& attribute_num)
{
	bool haveAttribute = false;
	int block_id = findTableBlock(table_name);
	if (block_id == -1) throw table_not_exist();
	else {
		// 装入
		char* pointer = buffer_manager.getPage(CATALOG_FILEPATH, block_id);
		std::vector<Catalog> catalog = decodeAllCatalog(pointer);
		// 找到并删除对应的catalog
		for (std::vector<Catalog>::iterator i = catalog.begin(); i != catalog.end() && haveAttribute == false; i++) {
			if ((*i).TableName == table_name) {	//找到table name
				for (int k = 0; k < (*i).attribute.num; k++) {
					if ((*i).attribute.name[k] == attribute_name) {
						haveAttribute = true;
						attribute_num = k;
						break;
					}
				}
			}
		}
	}
	return haveAttribute;
}

Attribute CatalogManager::getAttribute(std::string table_name)
{
	Catalog catalog = getCatalog(table_name);
	return catalog.attribute;
}

Index CatalogManager::getIndex(std::string table_name)
{
	Catalog catalog = getCatalog(table_name);
	return catalog.index;
}

void CatalogManager::addIndex(std::string table_name, std::string attribute_name, std::string index_name)
{

	int block_id = findTableBlock(table_name);
	if (block_id == -1) throw table_not_exist();
	else {
		// 装入
		char* pointer = buffer_manager.getPage(CATALOG_FILEPATH, block_id);
		std::vector<Catalog> exist_catalogs = decodeAllCatalog(pointer);	//得到block中现有的catalog
		for (size_t j = 0; j < exist_catalogs.size(); j++) {
			if (exist_catalogs[j].TableName == table_name) {	//找到table_name
				//开始寻找属性的序号
				int k = 0;
				for (k = 0; k < exist_catalogs[j].attribute.num; k++) {
					if (exist_catalogs[j].attribute.name[k] == attribute_name) {
						//判断index是否已经存在
						if (exist_catalogs[j].attribute.index[k] == true) throw index_exist_conflict();
						//判断属性是否unique
						if (exist_catalogs[j].attribute.unique[k] == false) throw attribute_not_unique();
						//判断索引名是否重复
						for (int m = 0; m < exist_catalogs[j].index.indexNumber; m++) {
							if (exist_catalogs[j].index.indexname[m] == index_name) throw index_name_conflict();
						}
						//冲突不存在，添加索引记录
						exist_catalogs[j].attribute.index[k] = true;
						exist_catalogs[j].index.indexname[exist_catalogs[j].index.indexNumber] = index_name;
						exist_catalogs[j].index.location[exist_catalogs[j].index.indexNumber] = k;
						exist_catalogs[j].index.indexNumber++;
						//重新刷入
						// 将catalog重写写回
						// 先建立块头
						std::string blockhead = createFormatedBlockString();
						for (size_t i = 0; i < blockhead.size(); i++) pointer[i] = blockhead[i];
						// 开始写入
						for (size_t i = 0; i < exist_catalogs.size(); i++) {
							std::string encodedstring = encodeCatalog(exist_catalogs[i]);
							writeTableString(pointer, encodedstring);
						}
						buffer_manager.setDirty(buffer_manager.getPageId(CATALOG_FILEPATH, block_id));
						return;
					}
				}
				if (k == exist_catalogs[j].attribute.num) throw attribute_not_exist();
			}
		}
	}
}

void CatalogManager::dropIndex(std::string table_name, std::string index_name)
{

	int block_id = findTableBlock(table_name);
	if (block_id == -1) throw table_not_exist();
	else {
		//装入
		char* pointer = buffer_manager.getPage(CATALOG_FILEPATH, block_id);
		std::vector<Catalog> exist_catalogs = decodeAllCatalog(pointer);	//得到block中现有的catalog
		for (size_t j = 0; j < exist_catalogs.size(); j++) {
			if (exist_catalogs[j].TableName == table_name) {	//找到table_name
				//判断index是否存在
				for (int k = 0; k < exist_catalogs[j].index.indexNumber; k++) {
					if (exist_catalogs[j].index.indexname[k] == index_name) {	//找到index_name
						exist_catalogs[j].attribute.index[exist_catalogs[j].index.location[k]] = false;
						exist_catalogs[j].index.indexname[k] = exist_catalogs[j].index.indexname[exist_catalogs[j].index.indexNumber - 1];
						exist_catalogs[j].index.location[k] = exist_catalogs[j].index.location[exist_catalogs[j].index.indexNumber - 1];
						exist_catalogs[j].index.indexNumber--;	//将最后一个记录覆盖到当前位置，以删除当前位置记录
						// 重新刷入table
						// 将catalog重写写回
						// 先建立块头
						std::string blockhead = createFormatedBlockString();
						for (size_t i = 0; i < blockhead.size(); i++) pointer[i] = blockhead[i];
						// 开始写入
						for (size_t i = 0; i < exist_catalogs.size(); i++) {
							std::string encodedstring = encodeCatalog(exist_catalogs[i]);
							writeTableString(pointer, encodedstring);
						}
						buffer_manager.setDirty(buffer_manager.getPageId(CATALOG_FILEPATH, block_id));
						return;
					}
				}
				throw index_not_exist();
			}
		}
	}
}

void CatalogManager::createCatalogFile()
{
	int blocknum = 2;	//第一块记录blocknum信息，记录储存从第二块开始
	std::string empty_block = createFormatedBlockString();
	std::string table_path = CATALOG_FILEPATH;
	std::ofstream fs(table_path);
	fs.write("####", sizeof(int));	//写入文件头
	fs.write((char*)&blocknum, sizeof(int));
	for (size_t i = 0; i < PAGESIZE - 2 * sizeof(int); i++)
		fs.write("\0", 1);	//补充至PAGESIZE
	fs << empty_block;	//在第二块写入块头信息
}

std::string CatalogManager::createFormatedBlockString()
{
	std::stringstream s;
	char init[sizeof(int) + 1] = "####";
	int size = 12;	//一块block的前4个bytes存block总占用空间
	int TupleNumber = 0;	//后4个bytes存block有多少个Table，随后Table按顺序密集存放
	s.write(init, sizeof(int));
	s.write((char*)&size, sizeof(int));
	s.write((char*)&TupleNumber, sizeof(int));
	return s.str();
}

int CatalogManager::getBlockNumber()
{
	int block_id = 0;
	char* first_page = buffer_manager.getPage(CATALOG_FILEPATH, 0);
	memcpy_s(&block_id, sizeof(int), first_page + sizeof(int), sizeof(int));
	return block_id;
}

void CatalogManager::setBlockNumber(int block_number)
{
	char* first_page = buffer_manager.getPage(CATALOG_FILEPATH, 0);
	memcpy_s(first_page + sizeof(int), sizeof(int), &block_number, sizeof(int));
	buffer_manager.setDirty(buffer_manager.getPageId(CATALOG_FILEPATH, 0));
}

void CatalogManager::setBlockStringSize(int size, char* pointer)
{
	pointer += sizeof(int);
	memcpy_s(pointer, sizeof(int), &size, sizeof(int));
}

void CatalogManager::setTableStringSize(int size, char* pointer)
{
	pointer += sizeof(int) * 2;
	memcpy_s(pointer, sizeof(int), &size, sizeof(int));
}

int CatalogManager::getBlockStringSize(char* pointer)
{
	int size = 0;
	pointer += sizeof(int);
	memcpy_s(&size, sizeof(int), pointer, sizeof(int));
	return size;
}

int CatalogManager::getTableNum(char* pointer)
{
	int size = 0;
	pointer += sizeof(int) * 2;
	memcpy_s(&size, sizeof(int), pointer, sizeof(int));
	return size;
}

void CatalogManager::writeTableString(char* pointer, std::string& encoded_tuple)
{
	int old_size = getBlockStringSize(pointer);
	char* end = pointer + old_size;
	for (size_t i = 0; i < encoded_tuple.size(); i++) end[i] = encoded_tuple[i];	//写入新的Table
	setBlockStringSize(old_size + encoded_tuple.size(), pointer);
	setTableStringSize(getTableNum(pointer) + 1, pointer);	//设置BlockSize和Table个数
}

std::string CatalogManager::encodeCatalog(Catalog& catalog)
{
	std::stringstream s;
	// 写入表名大小和表名
	int table_name_size = (int)catalog.TableName.size();
	s.write((char*)&table_name_size, sizeof(int));
	s.write(catalog.TableName.c_str(), table_name_size);
	// 写入属性数量
	s.write((char*)&(catalog.attribute.num), sizeof(int));
	// 写入属性的primary_key
	s.write((char*)&(catalog.attribute.primary_key), sizeof(int));
	// 写入属性信息
	for (int i = 0; i < catalog.attribute.num; i++) {
		// 写入属性名的大小和属性名
		int attrNameSize = catalog.attribute.name[i].size();
		s.write((char*)&attrNameSize, sizeof(int));
		s.write(catalog.attribute.name[i].c_str(), attrNameSize);
		// 写入属性type
		s.write((char*)&(catalog.attribute.type[i]), sizeof(int));
		// 写入unique
		s.write((char*)&(catalog.attribute.unique[i]), sizeof(bool));
		// 写入index
		s.write((char*)&(catalog.attribute.index[i]), sizeof(bool));
	}
	// 写入index数量
	s.write((char*)&(catalog.index.indexNumber), sizeof(int));
	// 写入index信息
	for (int i = 0; i < catalog.index.indexNumber; i++) {
		// 写入location
		s.write((char*)&(catalog.index.location[i]), sizeof(int));
		// 写入indexname
		int indexNameLen = catalog.index.indexname[i].size();
		s.write((char*)&indexNameLen, sizeof(int));
		s.write(catalog.index.indexname[i].c_str(), indexNameLen);
	}
	return s.str();
}

Catalog CatalogManager::decodeSingleCatalog(char* page_pointer, int& offset)
{
	Catalog catalog;
	// 读取tablename
	int table_name_size;
	memcpy_s(&table_name_size, sizeof(int), page_pointer + offset, sizeof(int));
	offset += sizeof(int);
	for (int i = 0; i < table_name_size; i++) {
		catalog.TableName += page_pointer[offset];
		offset++;
	}
	// 读取属性数量
	memcpy_s(&(catalog.attribute.num), sizeof(int), page_pointer + offset, sizeof(int));
	offset += sizeof(int);
	// 读取属性的primary_key
	memcpy_s(&(catalog.attribute.primary_key), sizeof(int), page_pointer + offset, sizeof(int));
	offset += sizeof(int);
	for (int i = 0; i < catalog.attribute.num; i++) {
		// 读取属性名的大小和属性名
		int attrNameSize;
		memcpy_s(&attrNameSize, sizeof(int), page_pointer + offset, sizeof(int));
		offset += sizeof(int);
		for (int j = 0; j < attrNameSize; j++) {
			catalog.attribute.name[i] += page_pointer[offset];
			offset++;
		}
		// 读取属性type
		memcpy_s(&(catalog.attribute.type[i]), sizeof(int), page_pointer + offset, sizeof(int));
		offset += sizeof(int);
		// 读取unique
		memcpy_s(&(catalog.attribute.unique[i]), sizeof(bool), page_pointer + offset, sizeof(bool));
		offset += sizeof(bool);
		// 读取index
		memcpy_s(&(catalog.attribute.index[i]), sizeof(bool), page_pointer + offset, sizeof(bool));
		offset += sizeof(bool);
	}
	// 读取index数量
	memcpy_s(&(catalog.index.indexNumber), sizeof(int), page_pointer + offset, sizeof(int));
	offset += sizeof(int);
	// 读取index信息
	for (int i = 0; i < catalog.index.indexNumber; i++) {
		// 读取location
		memcpy_s(&(catalog.index.location[i]), sizeof(int), page_pointer + offset, sizeof(int));
		offset += sizeof(int);
		// 读取indexname
		int indexNameLen;
		memcpy_s(&indexNameLen, sizeof(int), page_pointer + offset, sizeof(int));
		offset += sizeof(int);
		for (int j = 0; j < indexNameLen; j++) {
			catalog.index.indexname[i] += page_pointer[offset];
			offset++;
		}
	}
	return catalog;
}

std::vector<Catalog> CatalogManager::decodeAllCatalog(char* pointer)
{
	std::vector<Catalog> allCatalog;
	int offset = 3 * sizeof(int);
	int tuple_num = getTableNum(pointer);
	for (int i = 0; i < tuple_num; i++) {
		allCatalog.push_back(decodeSingleCatalog(pointer, offset));
	}
	return allCatalog;
}
