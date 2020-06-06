#include "CatalogManager.h"

//int main() {
	//CatalogManager m;
	//Catalog new_catalog;	//建立new_catalog对象
	//new_catalog.TableName = "A";
	//new_catalog.attribute.attributeNumber = 2;
	//new_catalog.attribute.type[0] = INT;
	//new_catalog.attribute.type[1] = FLOAT;
	//new_catalog.attribute.name[0] = "Num";
	//new_catalog.attribute.name[1] = "Cal";
	//new_catalog.attribute.unique[0] = false;
	//new_catalog.attribute.unique[1] = false;
	//new_catalog.attribute.index[0] = false;
	//new_catalog.attribute.index[1] = false;
	//Index index;
	//new_catalog.index = index;	//给new_catalog对象赋值
	//
	//m.createTable(new_catalog.TableName, new_catalog.attribute, new_catalog.index);
	//m.dropTable(new_catalog.TableName);
	//std::cout<<m.havetable(new_catalog.TableName);
	
	//m.showCatalog("E");

	//std::cout<<m.haveAttribute(new_catalog.TableName, "Num1");

	//m.addIndex(new_catalog.TableName, "Cal", "Calculation");

	//int page_id = catalog::buffer.loadBlock(0);
	//std::vector<Catalog> v = m.loadCatalogFromPage(page_id);
	//v.push_back(new_catalog);
	//m.flushCatalogToPage(v, page_id);
	//std::vector <Catalog> abc = m.loadCatalogFromPage(page_id);
//	return 0;
//}

//Catalog::Catalog(const Catalog& catalog)
//{
//	this->TableName = catalog.TableName;
//	this->attribute = catalog.attribute;
//	this->index = catalog.index;
//}

void CatalogManager::createTable(std::string table_name, Attribute attribute, Index index)
{
	//这里判断是否存在表名相同的表。存在则抛出table_name_conflict异常
	if (havetable(table_name)) throw table_name_conflict();

	Catalog new_catalog;	//建立new_catalog对象
	new_catalog.TableName = table_name;
	new_catalog.attribute = attribute;
	new_catalog.index = index;	//给new_catalog对象赋值
	//从第一个block开始搜索
	int i;
	for (i = 0; i < catalog::buffer.getBlockNum(); i++) {
		int page_id = catalog::buffer.getPageID(i);
		if (page_id == -1) page_id = catalog::buffer.loadBlock(i);	//加载block i
		std::vector<Catalog> exist_catalogs = loadCatalogFromPage(page_id);	//得到block中现有的catalog
		//如果block容量足够，则将新的catalog写入这个block
		if (catalogSize(exist_catalogs) + sizeof(new_catalog) + 4 < PAGESIZE) {
			exist_catalogs.push_back(new_catalog);
			flushCatalogToPage(exist_catalogs, page_id);	//新catalog写入Page
			//catalog::buffer.swapOutPage(page_id);	//Page写入Block
			return;
		}
	}
	//如果现有block的空间都不够，则在一个新的block上加入记录
	if (i == catalog::buffer.getBlockNum()) {
		int page_id = catalog::buffer.loadBlock(i);	//加载block i（新的块）
		std::vector<Catalog> exist_catalogs = loadCatalogFromPage(page_id);	//得到block中现有的catalog
		//如果block容量足够，则将新的catalog写入这个block
		if (catalogSize(exist_catalogs) + sizeof(new_catalog) + 4 < PAGESIZE) {
			exist_catalogs.push_back(new_catalog);
			flushCatalogToPage(exist_catalogs, page_id);	//新catalog写入Page
			//catalog::buffer.swapOutPage(page_id);	//Page写入Block
			return;
		}
	}
}

bool CatalogManager::havetable(std::string table_name)
{
	bool havetable = false;
	//从第一个block开始搜索
	for (int i = 0; i < catalog::buffer.getBlockNum() && havetable == false; i++) {
		int page_id = catalog::buffer.getPageID(i);
		if (page_id == -1) page_id = catalog::buffer.loadBlock(i);	//加载block i
		std::vector<Catalog> exist_catalogs = loadCatalogFromPage(page_id);	//得到block中现有的catalog
		for (size_t j = 0; j < exist_catalogs.size(); j++) {
			if (exist_catalogs[j].TableName == table_name) {
				//存在同名表
				havetable = true;
				break;
			}
		}
	}
	return havetable;
}

int CatalogManager::findTableBlock(std::string table_name)
{
	//从第一个block开始搜索
	for (int i = 0; i < catalog::buffer.getBlockNum(); i++) {
		int page_id = catalog::buffer.getPageID(i);
		if (page_id == -1) page_id = catalog::buffer.loadBlock(i);	//加载block i
		std::vector<Catalog> exist_catalogs = loadCatalogFromPage(page_id);	//得到block中现有的catalog
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
	//装入
	int page_id = catalog::buffer.getPageID(block_id);
	if (page_id == -1) page_id = catalog::buffer.loadBlock(block_id);	//加载block i
	std::vector<Catalog> exist_catalogs = loadCatalogFromPage(page_id);	//得到block中现有的catalog
	for (size_t j = 0; j < exist_catalogs.size(); j++) {
		if (exist_catalogs[j].TableName == table_name) {	//找到table_name
			exist_catalogs[j] = exist_catalogs[exist_catalogs.size() - 1];
			exist_catalogs.pop_back();	//将最后一个记录覆盖到将要drop的记录上，然后pop掉最后多余的记录
			break;
		}
	}
	flushCatalogToPage(exist_catalogs, page_id);	//新catalog写入Page
}

Catalog CatalogManager::getCatalog(std::string table_name)
{
	int block_id = findTableBlock(table_name);
	if (block_id == -1) throw table_not_exist();
	else {
		//装入
		int page_id = catalog::buffer.getPageID(block_id);
		std::vector<Catalog> exist_catalogs = loadCatalogFromPage(page_id);	//得到block中现有的catalog集
		for (size_t j = 0; j < exist_catalogs.size(); j++) {
			if (exist_catalogs[j].TableName == table_name) {	//找到table_name
				return exist_catalogs[j];
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
	for (int i = 0; i < catalog.attribute.attributeNumber; i++) {	//先扫描attribute获取name最大长度
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
	for (int i = 0; i < catalog.attribute.attributeNumber; i++) {
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
	size_t attr_max_length = max_length > 15 ? max_length:15;
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
		std::cout << catalog.attribute.name[catalog.index.num[i]];
		std::cout << std::endl;
	}
	for (size_t i = 0; i < max_length + attr_max_length + 2; i++) std::cout << "-";
	std::cout << std::endl;
}

bool CatalogManager::haveAttribute(std::string table_name, std::string attribute_name)
{
	bool haveAttribute = false;
	int block_id = findTableBlock(table_name);
	if (block_id == -1) throw table_not_exist();
	else {
		//装入
		int page_id = catalog::buffer.getPageID(block_id);
		std::vector<Catalog> exist_catalogs = loadCatalogFromPage(page_id);	//得到block中现有的catalog
		for (size_t j = 0; j < exist_catalogs.size() && haveAttribute == false; j++) {
			if (exist_catalogs[j].TableName == table_name) {	//找到table_name
				for (int k = 0; k < exist_catalogs[j].attribute.attributeNumber; k++) {
					if (exist_catalogs[j].attribute.name[k] == attribute_name) {
						haveAttribute = true;
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

void CatalogManager::addIndex(std::string table_name, std::string attribute_name, std::string index_name)
{
	int block_id = findTableBlock(table_name);
	if (block_id == -1) throw table_not_exist();
	else {
		//装入
		int page_id = catalog::buffer.getPageID(block_id);
		std::vector<Catalog> exist_catalogs = loadCatalogFromPage(page_id);	//得到block中现有的catalog
		for (size_t j = 0; j < exist_catalogs.size(); j++) {
			if (exist_catalogs[j].TableName == table_name) {	//找到table_name
				//开始寻找属性的序号
				int k = 0;
				for (k = 0; k < exist_catalogs[j].attribute.attributeNumber; k++) {
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
						exist_catalogs[j].index.num[exist_catalogs[j].index.indexNumber] = k;
						exist_catalogs[j].index.indexNumber++;
						break;
					}
				}
				if (k == exist_catalogs[j].attribute.attributeNumber) throw attribute_not_exist();
				flushCatalogToPage(exist_catalogs, page_id);	//新catalog写入Page 
			}
		}
	}
}

std::string CatalogManager::findAttributeThroughIndex(std::string table_name, std::string index_name)
{
	Catalog catalog = getCatalog(table_name);
	int i = 0;
	for (i = 0; i < catalog.index.indexNumber; i++) {
		if (index_name == catalog.index.indexname[i]) {
			return catalog.attribute.name[catalog.index.num[i]];
		}
	}
	if (i == catalog.index.indexNumber) throw index_not_exist();
	return std::string();
}

void CatalogManager::dropIndex(std::string table_name, std::string index_name)
{
	int block_id = findTableBlock(table_name);
	if (block_id == -1) throw table_not_exist();
	else {
		//装入
		int page_id = catalog::buffer.getPageID(block_id);
		std::vector<Catalog> exist_catalogs = loadCatalogFromPage(page_id);	//得到block中现有的catalog
		for (size_t j = 0; j < exist_catalogs.size(); j++) {
			if (exist_catalogs[j].TableName == table_name) {	//找到table_name
				//判断index是否存在
				for (int k = 0; k < exist_catalogs[j].index.indexNumber; k++) {
					if (exist_catalogs[j].index.indexname[k] == index_name) {	//找到index_name
						exist_catalogs[j].attribute.index[exist_catalogs[j].index.num[k]] = false;
						exist_catalogs[j].index.indexname[k] = exist_catalogs[j].index.indexname[exist_catalogs[j].index.indexNumber - 1];
						exist_catalogs[j].index.num[k] = exist_catalogs[j].index.num[exist_catalogs[j].index.indexNumber - 1];
						exist_catalogs[j].index.indexNumber--;	//将最后一个记录覆盖到当前位置，以删除当前位置记录
						return;
					}
				}
				throw index_not_exist();
			}
		}
	}
}

std::vector<Catalog> CatalogManager::loadCatalogFromPage(int page_id)
{
	char* page_pointer = catalog::buffer.getPagePointer(page_id);	//得到页地址
	std::vector<Catalog> catalog;	//创建空的catalog集合
	//从头开始搜索
	for (int i = 0; i < PAGESIZE; i++) {
		if (page_pointer[i] == ENTITY_DIVIDER[0] && page_pointer[i+1] == ENTITY_DIVIDER[1] && page_pointer[i+2] == ENTITY_DIVIDER[2] && page_pointer[i+3] == ENTITY_DIVIDER[3]) {
			Catalog* p = (Catalog*)(page_pointer + i + 4);
			catalog.push_back(*p);
		}
	}
	return catalog;
}

int CatalogManager::catalogSize(std::vector<Catalog>& catalog)
{
	int size = 0;
	for (size_t i = 0; i < catalog.size(); i++) {
		size += 4;	// + ENTITY_DIVIDER的长度
		size += sizeof(catalog[i]);	// + 一个catalog对象的大小
	}
	return size;
}

void CatalogManager::flushCatalogToPage(std::vector<Catalog>& catalog, int page_id)
{
	char* page_pointer = catalog::buffer.getPagePointer(page_id);

	char entity[PAGESIZE];
	char* p = entity;
	memset(entity, 0, PAGESIZE);	//创建临时entity
	for (size_t i = 0; i < catalog.size(); i++) {
		memcpy_s(p, 4, ENTITY_DIVIDER, 4);
		p += 4;	//前进四位
		memcpy_s(p, (int)(entity - p) + PAGESIZE, &(catalog[i]), sizeof(catalog[i]));
		p += sizeof(catalog[i]);	//p前进sizeof(catalog[i])
	}
	
	memcpy_s(page_pointer, PAGESIZE, entity, PAGESIZE);	//写入page
	catalog::buffer.setDirty(page_id, true);	//设置脏页标记
}
