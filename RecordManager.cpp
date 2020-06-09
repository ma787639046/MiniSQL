#include "RecordManager.h"

//int main() {
//	RecordManager m;
//	Tuple tuple;
//	key_ key;
//	key.INT_VALUE = 0;
//	tuple.addTuple(key);
//	key.FLOAT_VALUE = 2.1f;
//	tuple.addTuple(key);
//	key.STRING_VALUE = "ABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGH";
//	tuple.addTuple(key);
//
//	return 0;
//}

void RecordManager::createTableFile(std::string table_name)
{
	table_name = RECORD_PATH + table_name + ".db";
	FILE* f = fopen(table_name.c_str(), "w");
	fclose(f);
	std::string empty_block = createFormatedBlockString();
	flushToBlock(empty_block, 0, table_name);	//将FormatedBlock刷入文件头
}

void RecordManager::dropTableFile(std::string table_name)
{
	table_name = RECORD_PATH + table_name + ".db";
	remove(table_name.c_str());
}

int RecordManager::deleteRecord(std::string table_name)
{
	int deleteNum = 0;	//统计删除了多少条记录
	// 判断对应表名是否存在
	CatalogManager* catalog_manager = new CatalogManager();
	if (!catalog_manager->havetable(table_name)) throw table_not_exist();
	delete(catalog_manager);
	// 获取总block数量（这里不用担心储存record的db文件被删除，因为getBlockNum()会返回0）
	BufferManager* buffer_manager = new BufferManager(table_name, 1);
	int block_number = buffer_manager->getBlockNum();
	delete(buffer_manager);
	// 从第一个block开始，装入Tuple
	for (int i = 0; i < block_number; i++) {
		std::string s = loadBlockString(i, table_name);	//装入block
		deleteNum += getTupleNum(s);	//获取当前block有多少条记录
		s = createFormatedBlockString();	//创建空的formated string
		flushToBlock(s, i, table_name);	//刷入对应page中
	}
	return deleteNum;
}

int RecordManager::deleteRecord(std::string table_name, std::vector<Relation> relation)
{
	int deleteNum = 0;	//统计删除了多少条tuple
	std::vector<int> index;	//按顺序储存relation中，属性的序号
	int temp = 0;
	// 判断对应表名是否存在
	CatalogManager* catalog_manager = new CatalogManager();
	if (!catalog_manager->havetable(table_name)) throw table_not_exist();
	for (size_t i = 0; i < relation.size(); i++) {
		if (!catalog_manager->haveAttribute(table_name, relation[i].attributeName, temp)) throw attribute_not_exist();
		index.push_back(temp);	//存入属性的序号
	}
	Attribute attribute = catalog_manager->getAttribute(table_name);	//获取表的所有属性信息
	delete(catalog_manager);
	// 检查Attribute的类型与输入的relation是否匹配
	for (size_t i = 0; i < relation.size(); i++) {
		if (attribute.type[index[i]] != relation[i].attributeType) throw key_type_conflict();
	}
	// 获取总block数量（这里不用担心储存record的db文件被删除，因为getBlockNum()会返回0）
	BufferManager* buffer_manager = new BufferManager(table_name, 1);
	int block_number = buffer_manager->getBlockNum();
	delete(buffer_manager);

	//异常检查完毕，开始遍历record，寻找符合条件的tuple
	std::string formatedString;
	formatedString.reserve(PAGESIZE);
	for (int block_id = 0; block_id < block_number; block_id++) {
		formatedString = loadBlockString(block_id, table_name);
		std::vector<Tuple> current_record = decodeTupleString(formatedString);	//获得当前block中的tuples
		std::vector<Tuple> new_record;	//储存删除后的tuple
		//依次判断是否tuple是否满足relation
		for (size_t j = 0; j < current_record.size(); j++) {//对于每一个tuple
			std::vector<key_> current_tuple = current_record[j].getTuple();	//获得这条tuple
			bool meetRelation = true;
			for (size_t k = 0; k < relation.size(); k++) {//对于每一个relation
				meetRelation = this->meetRelation(current_tuple[index[k]], relation[k]);
				if (meetRelation == false) break;
			}
			if (meetRelation == false) {	//不满足所有条件，保留这条tuple
				new_record.push_back(current_record[j]);
			}
		}
		if (new_record.size() < current_record.size()) {//新的tuple数量变少，说明有删除情况
			std::string newFormatedString = createFormatedBlockString();
			for (size_t m = 0; m < new_record.size(); m++)
				writeTupleString(newFormatedString, new_record[m]);	//创建新的formatedString
			flushToBlock(newFormatedString, block_id, table_name);	//将新tuple刷入对应block_id的page中
			deleteNum += current_record.size() - new_record.size();	//统计删了几条tuple
		}		
	}
	return deleteNum;
}

void RecordManager::insertRecord(std::string table_name, Tuple& tuple)
{
	// 判断对应表名是否存在
	CatalogManager* catalog_manager = new CatalogManager();
	if (!catalog_manager->havetable(table_name)) throw table_not_exist();
	Attribute attribute = catalog_manager->getAttribute(table_name);	//获取表的所有属性信息
	delete(catalog_manager);
	// 判断主键插入是否重复
	std::vector<key_> keys = tuple.getTuple();	//获取一条tuple
	if (attribute.primary_key != -1) {
		Relation search_relation;
		search_relation.attributeName = attribute.name[attribute.primary_key];	//获得主键属性名
		if (attribute.type[attribute.primary_key] == INT) 
			search_relation.key.INT_VALUE = keys[attribute.primary_key].INT_VALUE;
		else if (attribute.type[attribute.primary_key] == FLOAT)
			search_relation.key.FLOAT_VALUE = keys[attribute.primary_key].FLOAT_VALUE;
		else if (attribute.type[attribute.primary_key] == STRING)
			search_relation.key.STRING_VALUE = keys[attribute.primary_key].STRING_VALUE;	//获得主键值
		search_relation.sign = EQUAL;	//等于
		std::vector<Relation> search_relation_vector;
		search_relation_vector.push_back(search_relation);
		std::vector<Tuple> record = loadRecord(table_name, search_relation_vector);	//搜索是否存在相同主键
		if (record.size() > 0) throw primary_key_conflict();	//存在重复主键，抛出primary_key_conflict
		search_relation_vector.clear();
	}
	// 判断unique值是否重复
	if (attribute.attributeNumber != keys.size()) throw key_type_conflict();	//传入的tuple中key个数与attribute个数不一致，抛出key_type_conflict
	for (size_t i = 0; i < attribute.attributeNumber; i++) {
		if (attribute.unique[i] == true) {
			// 第i个属性为unique，需要判断unique值是否存在重复
			Relation search_relation;
			search_relation.attributeName = attribute.name[i];	//获得unique key的属性名
			if (attribute.type[i] == INT)
				search_relation.key.INT_VALUE = keys[i].INT_VALUE;
			else if (attribute.type[i] == FLOAT)
				search_relation.key.FLOAT_VALUE = keys[i].FLOAT_VALUE;
			else if (attribute.type[i] == STRING)
				search_relation.key.STRING_VALUE = keys[i].STRING_VALUE;	//获得unique key的值
			search_relation.sign = EQUAL;	//等于
			std::vector<Relation> search_relation_vector;
			search_relation_vector.push_back(search_relation);
			std::vector<Tuple> record = loadRecord(table_name, search_relation_vector);	//搜索是否存在相同unique key
			if (record.size() > 0) throw unique_conflict();	//存在重复主键，抛出unique_conflict
			search_relation_vector.clear();
		}
	}
	//异常检查完毕，开始插入这条tuple
	std::string table_path = RECORD_PATH + table_name + ".db";
	BufferManager * bufffer_manager = new BufferManager(table_path, 1);	//因为只需获取block number，所以无需较多page
	int blocknum = bufffer_manager->getBlockNum();
	delete(bufffer_manager);
	std::string formatedString;
	formatedString.reserve(PAGESIZE);
	for (size_t i = 0; i < blocknum; i++) {
		formatedString = loadBlockString(i, table_name);
		if (getBlockStringSize(formatedString) + getTupleSize(tuple) < PAGESIZE) {//空间足够
			writeTupleString(formatedString, tuple);	//写入tuple
			flushToBlock(formatedString, i, table_name);	//将新tuple刷入对应block_id的page中
			return;
		}
	}
	//所有block均无足够大的空间，容纳新的tuple，则在新的block储存tuple
	formatedString = createFormatedBlockString();	//格式化formatedString
	if (getBlockStringSize(formatedString) + getTupleSize(tuple) < PAGESIZE) {
		writeTupleString(formatedString, tuple);	//写入tuple
		flushToBlock(formatedString, blocknum, table_name);	//将新tuple刷入对应block_id = blocknum的page中
	}
	else throw tuple_out_of_range();	//tuple太大了，抛出tuple_out_of_range
}

std::vector<Tuple> RecordManager::loadRecord(std::string table_name)
{
	// 判断对应表名是否存在
	CatalogManager* catalog_manager = new CatalogManager();
	if (!catalog_manager->havetable(table_name)) throw table_not_exist();
	delete(catalog_manager);
	// 获取总block数量（这里不用担心储存record的db文件被删除，因为getBlockNum()会返回0）
	BufferManager* buffer_manager = new BufferManager(table_name, 1);
	int block_number = buffer_manager->getBlockNum();
	delete(buffer_manager);
	// 从第一个block开始，装入Tuple
	std::vector<Tuple> tuple;
	for (int i = 0; i < block_number; i++) {
		std::string s = loadBlockString(i, table_name);	//装入block
		std::vector<Tuple> temp = decodeTupleString(s);	//decode blockString为vector<Tuple>
		for (int j = 0; j < temp.size(); j++)
			tuple.push_back(temp[i]);
		temp.clear();
	}
	return tuple;
}

std::vector<Tuple> RecordManager::loadRecord(std::string table_name, std::vector<Relation> relation)
{
	std::vector<int> index;	//按顺序储存relation中，属性的序号
	int temp = 0;
	// 判断对应表名是否存在
	CatalogManager* catalog_manager = new CatalogManager();
	if (!catalog_manager->havetable(table_name)) throw table_not_exist();
	for (size_t i = 0; i < relation.size(); i++) {
		if (!catalog_manager->haveAttribute(table_name, relation[i].attributeName, temp)) throw attribute_not_exist();
		index.push_back(temp);	//存入属性的序号
	}
	Attribute attribute = catalog_manager->getAttribute(table_name);	//获取表的所有属性信息
	delete(catalog_manager);
	// 检查Attribute的类型与输入的relation是否匹配
	for (size_t i = 0; i < relation.size(); i++) {
		if (attribute.type[index[i]] != relation[i].attributeType) throw key_type_conflict();
	}
	// 获取总block数量（这里不用担心储存record的db文件被删除，因为getBlockNum()会返回0）
	BufferManager* buffer_manager = new BufferManager(table_name, 1);
	int block_number = buffer_manager->getBlockNum();
	delete(buffer_manager);
	// 从第一个block开始，装入Tuple
	std::vector<Tuple> tuple;
	for (int i = 0; i < block_number; i++) {
		std::string s = loadBlockString(i, table_name);	//装入block
		std::vector<Tuple> temp = decodeTupleString(s);	//decode blockString为vector<Tuple>
		for (size_t j = 0; j < temp.size(); j++) {
			//判断每个读入的tuple，是否满足relation
			bool meetRelation = true;	//满足条件设为true，不满足设为false
			std::vector<key_> keys = temp[j].getTuple();	//获取这条record的所有key
			for (size_t m = 0; m < relation.size(); m++) {
				meetRelation = this->meetRelation(keys[index[m]], relation[m]);
				if (meetRelation == false) break;	//如果这条记录中不满足某一个条件，则跳出判断循环
			}
			if (meetRelation) tuple.push_back(temp[i]);	//满足所有条件，则存入这个记录
			keys.clear();	//清空keys，用于下次使用
		}
		temp.clear();
	}
	return tuple;
}

std::string RecordManager::createFormatedBlockString()
{
	std::stringstream s;
	int size = 8;	//一块block的前4个bytes存block总占用空间
	int TupleNumber = 0;	//后4个bytes存block有多少个Tuple，随后Tuple按顺序密集存放
	s.write((char*)&size, sizeof(int));
	s.write((char*)&TupleNumber, sizeof(int));
	return s.str();
}

int RecordManager::getBlockStringSize(std::string s)
{
	int size = 0;
	char* pointer = (char*)&size;
	for (int i = 0; i < sizeof(int); i++) {
		pointer[i] = s[i];
	}
	return size;
}

void RecordManager::setBlockStringSize(int size, std::string& s)
{
	char* pointer = (char*) &size;
	for (int i = 0; i < sizeof(int); i++) {
		s[i] = pointer[i];
	}
}

int RecordManager::getTupleNum(std::string s)
{
	int size = 0;
	char* pointer = (char*)&size;
	for (int i = 0; i < sizeof(int); i++) {
		pointer[i] = s[i + sizeof(int)];
	}
	return size;
}

void RecordManager::setTupleStringSize(int size, std::string& s)
{
	char* pointer = (char*)&size;
	for (int i = sizeof(int); i < 2 * sizeof(int); i++) {
		s[i] = pointer[i - sizeof(int)];
	}
}

void RecordManager::flushToBlock(std::string s, int block_id, std::string table_name)
{
	std::string filepath = RECORD_PATH + table_name + ".db";
	BufferManager buffer(filepath, 1);	//因为只需要刷一个block，因此需要1个page
	int page_id = buffer.loadBlock(block_id);	//装入block
	char* pointer = buffer.getPagePointer(page_id);	//得到page pointer
	//写入page
	for (int i = 0; i < s.size(); i++)
		pointer[i] = s[i];
	buffer.setDirty(page_id, true);	//设置脏页
}

std::string RecordManager::loadBlockString(int block_id, std::string table_name)
{
	std::string filename = RECORD_PATH + table_name + ".db";
	BufferManager buffer(filename, 1);	//因为只需要装一个block，因此需要1个page
	int page_id = buffer.loadBlock(block_id);	//装入block
	char* pointer = buffer.getPagePointer(page_id);	//得到page pointer
	std::string block;
	block.reserve(PAGESIZE);
	for (int i = 0; i < PAGESIZE; i++)
		block = block + pointer[i];
	return block;
}

void RecordManager::writeTupleString(std::string& s, Tuple& tuple)
{
	std::string encoded_tuple = tuple.encodeTuple();
	int old_size = getBlockStringSize(s);
	s = s.substr(0, old_size);
	for (size_t i = 0; i < encoded_tuple.size(); i++) {
		s = s + encoded_tuple[i];	//写入新的Tuple
	}
	int size = old_size + encoded_tuple.size();
	int tuple_number = getTupleNum(s) + 1;
	setBlockStringSize(size, s);
	setTupleStringSize(tuple_number, s);	//设置BlockSize和Tuple个数
}

int RecordManager::getTupleSize(Tuple tuple)
{
	std::string encoded_tuple = tuple.encodeTuple();
	return (int)encoded_tuple.size();
}

std::vector<Tuple> RecordManager::decodeTupleString(const std::string s)
{
	std::vector<Tuple> tuples;
	int tupleNum = getTupleNum(s);
	std::string s_temp = s.substr(2 * sizeof(int));
	
	for (int i = 0; i < tupleNum; i++) {
		Tuple* t = new(Tuple);
		(*t).decodeTuple(s_temp);
		tuples.push_back(*t);
		delete(t);
	}
	return tuples;
}

bool RecordManager::meetRelation(key_ key, Relation relation)
{
	bool meetRelation = false;
	//先判断tuple的类型
	if (relation.attributeType == INT) {
		switch (relation.sign) {
		case NOT_EQUAL:
			if (key.INT_VALUE != relation.key.INT_VALUE) meetRelation = true;
			break;
		case EQUAL:
			if (key.INT_VALUE == relation.key.INT_VALUE) meetRelation = true;
			break;
		case LESS:
			if (key.INT_VALUE < relation.key.INT_VALUE) meetRelation = true;
			break;
		case LESS_OR_EQUAL:
			if (key.INT_VALUE <= relation.key.INT_VALUE) meetRelation = true;
			break;
		case GREATER_OR_EQUAL:
			if (key.INT_VALUE >= relation.key.INT_VALUE) meetRelation = true;
			break;
		case GREATER:
			if (key.INT_VALUE > relation.key.INT_VALUE) meetRelation = true;
			break;
		default: break;
		}
	}
	else if (relation.attributeType == FLOAT) {
		switch (relation.sign) {
		case NOT_EQUAL:
			if (key.FLOAT_VALUE != relation.key.FLOAT_VALUE) meetRelation = true;
			break;
		case EQUAL:
			if (key.FLOAT_VALUE == relation.key.FLOAT_VALUE) meetRelation = true;
			break;
		case LESS:
			if (key.FLOAT_VALUE < relation.key.FLOAT_VALUE) meetRelation = true;
			break;
		case LESS_OR_EQUAL:
			if (key.FLOAT_VALUE <= relation.key.FLOAT_VALUE) meetRelation = true;
			break;
		case GREATER_OR_EQUAL:
			if (key.FLOAT_VALUE >= relation.key.FLOAT_VALUE) meetRelation = true;
			break;
		case GREATER:
			if (key.FLOAT_VALUE > relation.key.FLOAT_VALUE) meetRelation = true;
			break;
		default: break;
		}
	}
	else if (relation.attributeType == STRING) {
		switch (relation.sign) {
		case NOT_EQUAL:
			if (key.STRING_VALUE != relation.key.STRING_VALUE) meetRelation = true;
			break;
		case EQUAL:
			if (key.STRING_VALUE == relation.key.STRING_VALUE) meetRelation = true;
			break;
		case LESS:
			if (key.STRING_VALUE < relation.key.STRING_VALUE) meetRelation = true;
			break;
		case LESS_OR_EQUAL:
			if (key.STRING_VALUE <= relation.key.STRING_VALUE) meetRelation = true;
			break;
		case GREATER_OR_EQUAL:
			if (key.STRING_VALUE >= relation.key.STRING_VALUE) meetRelation = true;
			break;
		case GREATER:
			if (key.STRING_VALUE > relation.key.STRING_VALUE) meetRelation = true;
			break;
		default: break;
		}
	}
	return meetRelation;
}
