//
//	RecordManager.cpp
//	Created by MGY on 2020/06/07
//

#include "RecordManager.h"

void RecordManager::createTableFile(std::string table_name)
{
	int blocknum = 2;	//第一块记录blocknum信息，记录储存从第二块开始
	std::string empty_block = createFormatedBlockString();
	std::string table_path = RECORD_PATH + table_name + ".db";
	std::ofstream fs(table_path);
	fs.write("####", sizeof(int));	//写入文件头
	fs.write((char*)&blocknum, sizeof(int));
	for (size_t i = 0; i < PAGESIZE - 2 * sizeof(int); i++)
		fs.write("\0", 1);	//补充至PAGESIZE
	fs << empty_block;	//在第二块写入块头信息
}

void RecordManager::dropTableFile(std::string table_name)
{
	table_name = RECORD_PATH + table_name + ".db";
	remove(table_name.c_str());
}

void RecordManager::insertRecord(std::string table_name, Tuple tuple)
{
	// 判断对应表名是否存在
	CatalogManager catalog_manager;
	//IndexManager index_manager(table_name);
	static IndexManager index_manager(table_name);
	if (!catalog_manager.havetable(table_name)) throw table_not_exist();
	Attribute attribute = catalog_manager.getAttribute(table_name);	//获取表的所有属性信息
	std::vector<key_> keys = tuple.getKeys();
	// 判断Tuple与attribute类型是否一一对应
	for (size_t i = 0; i < keys.size(); i++) {
		if (keys[i].type != attribute.type[i])
			throw tuple_type_conflict();
	}
	// 预判断是否有Unique的值
	bool haveUnique = false;
	for (size_t i = 0; i < attribute.num; i++) {
		if (attribute.unique[i] == true) {
			haveUnique = true;
			break;
		}
	}
	// 根据haveUnique，决定是否进行unique检查
	// 预判断的目的是，如果没有unique，则节省了loadRecord(table_name)的消耗
	if (haveUnique) {
		// 判断unique值是否重复
		for (size_t i = 0; i < attribute.num; i++) {
			if (attribute.unique[i] == true) {
				if (attribute.index[i] == true) {
					// 这里根据索引来查找重复
					std::string path = "INDEX_FILE_" + attribute.name[i] + "_" + table_name;
					std::vector<int> my_elem;
					index_manager.find_range(path, keys[i], keys[i], my_elem, keys[i].type, keys[i].type);
					//int findblock = index_manager.find_index(path, keys[i], keys[i].type);
					if (my_elem.size() >= 1) {
						// 找到相同的值
						if (i == attribute.primary_key) throw primary_key_conflict();
						else throw unique_conflict();
					}
				}
				else {
					Table table = loadRecord(table_name);
					if (haveSameKey(table, i, keys[i])) {
						// 发现相同的值，抛出异常
						if (i == attribute.primary_key) throw primary_key_conflict();
						else throw unique_conflict();
					}
				}
			}
		}
	}
	// encode待插入的Tuple
	std::string encodedTuple = encodeTuple(tuple);
	// 获取总block数量
	int block_number = getBlockNumber(table_name);
	// 获得最后一个block的pointer
	std::string table_path = RECORD_PATH + table_name + ".db";
	char* page_pointer = buffer_manager.getPage(table_path, block_number - 1);
	// 判断空间是否充足
	if (getBlockStringSize(page_pointer) + encodedTuple.size() < PAGESIZE) {
		// 空间充足
		writeTupleString(page_pointer, encodedTuple);
		buffer_manager.setDirty(buffer_manager.getPageId(table_path, block_number - 1));
		for (size_t i = 0; i < attribute.num; i++) {
			if (attribute.index[i] == true) {
				// 插入key至对应的b+树
				std::string path = "INDEX_FILE_" + attribute.name[i] + "_" + table_name;
				index_manager.insert_index(path, keys[i], block_number - 1, keys[i].type);
			}
		}
	}
	else {
		// 空间不足，开新的页
		setBlockNumber(block_number + 1, table_name);	//块数+1
		page_pointer = buffer_manager.getPage(table_path, block_number);
		// 写入页头
		std::string empty_block = createFormatedBlockString();
		for (size_t i = 0; i < empty_block.size(); i++)
			page_pointer[i] = empty_block[i];
		// 写入tuple
		writeTupleString(page_pointer, encodedTuple);
		buffer_manager.setDirty(buffer_manager.getPageId(table_path, block_number));
		for (size_t i = 0; i < attribute.num; i++) {
			if (attribute.index[i] == true) {
				// 插入key至对应的b+树
				std::string path = "INDEX_FILE_" + attribute.name[i] + "_" + table_name;
				index_manager.insert_index(path, keys[i], block_number, keys[i].type);
			}
		}
	}
}

int RecordManager::deleteRecord(std::string table_name)
{
	int deleteNum = 0;	//统计删除了多少条记录
	// 判断对应表名是否存在
	CatalogManager catalog_manager;
	if (!catalog_manager.havetable(table_name)) throw table_not_exist();
	// 获取总block数量
	std::string filename = RECORD_PATH + table_name + ".db";
	int block_number = getBlockNumber(table_name);
	// 从第一个block开始，装入Tuple
	for (int i = 0; i < block_number; i++) {
		char* page_pointer = buffer_manager.getPage(filename, i);
		deleteNum += getTupleNum(page_pointer);	//获取当前block有多少条记录
		setTupleStringSize(0, page_pointer);	//清除所有记录
		setBlockStringSize(3 * sizeof(int), page_pointer);	//重置block size
		buffer_manager.setDirty(buffer_manager.getPageId(filename, i));		//设置脏页
	}
	// 清空索引
	IndexManager index_manager(table_name);
	Attribute attr = catalog_manager.getAttribute(table_name);
	Index index = catalog_manager.getIndex(table_name);
	for (int i = 0; i < index.indexNumber; i++) {
		if (attr.index[index.location[i]]) {
			std::string filename = "INDEX_FILE_" + attr.name[index.location[i]] + "_" + table_name;
			index_manager.drop_index(filename, (int)attr.type[index.location[i]]);
		}
	}
	return deleteNum;
}

int RecordManager::deleteRecord(std::string table_name, std::vector<Relation> relation)
{
	int deleteNum = 0;	//统计删除了多少条tuple
	std::vector<int> index;	//按顺序储存relation中，属性的序号
	// 判断对应表名是否存在
	CatalogManager catalog_manager;
	if (!catalog_manager.havetable(table_name)) throw table_not_exist();
	Attribute attr = catalog_manager.getAttribute(table_name);	//获取表的所有属性信息
	Index index_ = catalog_manager.getIndex(table_name);
	for (int i = 0, temp = 0; i < relation.size(); i++) {
		if (!catalog_manager.haveAttribute(table_name, relation[i].attributeName, temp)) throw attribute_not_exist();
		index.push_back(temp);	//存入属性的序号
	}
	// 检查Attribute的类型与输入的relation是否匹配
	for (size_t i = 0; i < relation.size(); i++) {
		if (attr.type[index[i]] != relation[i].key.type) throw key_type_conflict();
	}
	// 建立表对象
	Table table(table_name, attr);
	// 获取总block数量
	int block_number = getBlockNumber(table_name);
	// 从第二个block开始，装入Tuple
	std::vector<Tuple>& tuple = table.getTuple();
	std::string filename = RECORD_PATH + table_name + ".db";
	// 从第二个block开始，装入Tuple
	for (int i = 1; i < block_number; i++) {
		char* page_pointer = buffer_manager.getPage(filename, i);
		int offset = 3 * sizeof(int);
		// 获得tuple个数
		int tuple_num = getTupleNum(page_pointer);
		for (int i = 0; i < tuple_num; i++) {
			int ori_offset = offset;	//记录旧的offset
			Tuple temp = decodeSingleTuple(page_pointer, offset);	//获得一条tuple，offset移动至下条记录
			std::vector<key_> keys = temp.getKeys();
			bool isRelation = true;
			for (size_t m = 0; m < relation.size(); m++) {
				isRelation = meetRelation(keys[index[m]], relation[m]);
				if (isRelation == false) break;	//如果这条记录中不满足某一个条件，则跳出判断循环
			}
			// 满足条件，删除这个tuple
			if (isRelation) {
				deleteNum++;	//删除条目数+1
				for (int j = 0; j < getBlockStringSize(page_pointer) - offset && j < PAGESIZE; j++ ) {
					page_pointer[j + ori_offset] = page_pointer[offset + j];	//将后面的数据覆盖到前面
					
				}
				setTupleStringSize(getTupleNum(page_pointer) - 1, page_pointer);	//记录数 - 1
				setBlockStringSize(getBlockStringSize(page_pointer) - offset + ori_offset, page_pointer);	//设置block size
				buffer_manager.setDirty(buffer_manager.getPageId(filename, i));		//设置脏页
				offset = ori_offset;	//offset移回当前起始位置
			}
			// 满足条件，删除b+树索引中的这条记录
			if (isRelation && index_.indexNumber > 0) {
				IndexManager index_manager(table_name);
				for (int m = 0; m < index_.indexNumber; m++) {
					std::string indexfilename = "INDEX_FILE_" + attr.name[index_.location[m]] + "_" + table_name;
					index_manager.delete_index(indexfilename, keys[index_.location[m]], attr.type[index_.location[m]]);
				}
			}
		}
	}
	return deleteNum;
}

Table RecordManager::loadRecord(std::string table_name)
{
	// 判断对应表名是否存在
	CatalogManager catalog_manager;
	if (!catalog_manager.havetable(table_name)) throw table_not_exist();
	// 获得表属性
	Attribute attr = catalog_manager.getAttribute(table_name);
	// 建立表对象
	Table table(table_name, attr);
	// 获取总block数量
	int block_number = getBlockNumber(table_name);
	// 从第二个block开始，装入Tuple
	std::vector<Tuple>& tuple = table.getTuple();
	std::string filepath = RECORD_PATH + table_name + ".db";
	for (int i = 1; i < block_number; i++) {
		// 获得页指针
		char* page_pointer  = buffer_manager.getPage(filepath ,i);
		int offset = 3 * sizeof(int);
		// 获得tuple个数
		int tuple_num = getTupleNum(page_pointer);
		for (int i = 0; i < tuple_num; i++) {
			// 逐个写入table
			tuple.push_back(decodeSingleTuple(page_pointer, offset));
		}
	}
	return table;
}

Table RecordManager::loadRecord(std::string table_name, std::vector<Relation> relation)
{
	std::vector<int> index;	//按顺序储存relation中，属性的序号
	// 判断对应表名是否存在
	CatalogManager catalog_manager;
	if (!catalog_manager.havetable(table_name)) throw table_not_exist();
	Attribute attribute = catalog_manager.getAttribute(table_name);	//获取表的所有属性信息
	for (int i = 0, temp = 0; i < relation.size(); i++) {
		if (!catalog_manager.haveAttribute(table_name, relation[i].attributeName, temp)) throw attribute_not_exist();
		index.push_back(temp);	//存入属性的序号
	}
	// 检查Attribute的类型与输入的relation是否匹配
	for (size_t i = 0; i < relation.size(); i++) {
		if (attribute.type[index[i]] != relation[i].key.type) throw key_type_conflict();
	}
	// 建立表对象
	Table table(table_name, attribute);
	// 获取总block数量
	int block_number = getBlockNumber(table_name);
	// 从第二个block开始，装入Tuple
	std::vector<Tuple>& tuple = table.getTuple();
	std::string filepath = RECORD_PATH + table_name + ".db";
	// 判断能否使用带索引的搜索
	bool search_with_index = false;
	if (relation.size() == 1) {
		if (relation[0].sign != NOT_EQUAL && attribute.index[index[0]] == true) {
			search_with_index = true;
		}
	}
	else if (relation.size() == 2) {
		// 两个搜索条件，属性名要一致
		if (relation[0].attributeName == relation[1].attributeName && attribute.index[index[0]] == true) {
			if (relation[0].sign != NOT_EQUAL || relation[1].sign != NOT_EQUAL || relation[0].sign != EQUAL || relation[1].sign != EQUAL) {
				search_with_index = true;
			}
		}
	}
	if (search_with_index) {
		std::vector<int> block_id_range;
		searchWithIndex(table_name, block_id_range, relation);
		//
		clock_t begin, end;
		double total_time;
		begin = clock();
		if (block_id_range.size() > 0 && block_id_range[0] != -1) {	//index找到了range
			for (std::vector<int>::iterator i = block_id_range.begin(); i != block_id_range.end(); i++) {
				// 获得页指针
				char* page_pointer = buffer_manager.getPage(filepath, *i);
				int offset = 3 * sizeof(int);
				// 获得tuple个数
				int tuple_num = getTupleNum(page_pointer);
				for (int j = 0; j < tuple_num; j++) {
					Tuple temp = decodeSingleTuple(page_pointer, offset);	//获得一条tuple
					std::vector<key_> keys = temp.getKeys();
					bool isRelation = true;
					for (size_t m = 0; m < relation.size(); m++) {
						isRelation = meetRelation(keys[index[m]], relation[m]);
						if (isRelation == false) break;	//如果这条记录中不满足某一个条件，则跳出判断循环
					}
					// 逐个写入table
					if (isRelation) tuple.push_back(temp);
				}
			}
			end = clock();
			total_time = (double)(end - begin) / CLOCKS_PER_SEC;
			std::cout << "The time cost of this select is: " << total_time << " seconds.\n";
			return table;
		}
		else {
			//std::cout << "\n!!! Index cannot return results !!!\n\n";
			search_with_index = false;
		}
	}
	if (!search_with_index) {
		//
		clock_t begin, end;
		double total_time;
		begin = clock();
		// 从第二块开始遍历每个块
		for (int i = 1; i < block_number; i++) {
			// 获得页指针
			char* page_pointer = buffer_manager.getPage(filepath, i);
			int offset = 3 * sizeof(int);
			// 获得tuple个数
			int tuple_num = getTupleNum(page_pointer);
			for (int j = 0; j < tuple_num; j++) {
				Tuple temp = decodeSingleTuple(page_pointer, offset);	//获得一条tuple
				std::vector<key_> keys = temp.getKeys();
				bool isRelation = true;
				for (size_t m = 0; m < relation.size(); m++) {
					isRelation = meetRelation(keys[index[m]], relation[m]);
					if (isRelation == false) break;	//如果这条记录中不满足某一个条件，则跳出判断循环
				}
				// 逐个写入table
				if (isRelation) tuple.push_back(temp);
			}
		}
		end = clock();
		total_time = (double)(end - begin) / CLOCKS_PER_SEC;
		std::cout << "The time cost of this select is: " << total_time << " seconds.\n";
		return table;
	}
}



void RecordManager::searchWithIndex(std::string table_name, std::vector<int>& elem, std::vector<Relation> relation)
{
	static IndexManager index_manager(table_name);
	if (relation.size() == 2) {
		if (relation[0].sign == LESS || relation[0].sign == LESS_OR_EQUAL) {
			if (relation[1].sign == GREATER || relation[1].sign == GREATER_OR_EQUAL) {
				std::string path = "INDEX_FILE_" + relation[0].attributeName + "_" + table_name;
				index_manager.find_range(path, relation[1].key, relation[0].key, elem, relation[1].key.type, relation[0].key.type);
				return;
			}
		}
		else if (relation[0].sign == GREATER || relation[0].sign == GREATER_OR_EQUAL) {
			if (relation[1].sign == LESS || relation[1].sign == LESS_OR_EQUAL) {
				std::string path = "INDEX_FILE_" + relation[0].attributeName + "_" + table_name;
				index_manager.find_range(path, relation[0].key, relation[1].key, elem, relation[0].key.type, relation[1].key.type);
				return;
			}
		}
	}
	else if (relation.size() == 1) {
		key_ another_key;
		another_key.type = relation[0].key.type;
		another_key.INT_VALUE = INF;
		another_key.FLOAT_VALUE = INF;
		another_key.STRING_VALUE = "";
		if (relation[0].sign == LESS || relation[0].sign == LESS_OR_EQUAL) {
			another_key.INT_VALUE = -another_key.INT_VALUE;
			another_key.FLOAT_VALUE = -another_key.FLOAT_VALUE;
			std::string path = "INDEX_FILE_" + relation[0].attributeName + "_" + table_name;
			index_manager.find_range(path, another_key, relation[0].key, elem, another_key.type, relation[0].key.type);
		}
		else if (relation[0].sign == GREATER || relation[0].sign == GREATER_OR_EQUAL) {
			std::string path = "INDEX_FILE_" + relation[0].attributeName + "_" + table_name;
			index_manager.find_range(path, relation[0].key, another_key , elem, relation[0].key.type, relation[0].key.type);
		}
		else if (relation[0].sign == EQUAL) {
			std::string path = "INDEX_FILE_" + relation[0].attributeName + "_" + table_name;
			index_manager.find_range(path, relation[0].key, relation[0].key, elem, relation[0].key.type, relation[0].key.type);
			//int found_block_id = index_manager.find_index(path, relation[0].key, relation[0].key.type);
			//elem.push_back(found_block_id);
		}
	}
}

//generate index for record manager
void RecordManager::generate_index(IndexManager& index_manager, std::string table_name, std::string cur_attr_name)
{
	CatalogManager catalog_manager;
	Attribute attr = catalog_manager.getAttribute(table_name);
	//find the cur_attr_name
	int index = -1;
	for (int i = 0; i < attr.num; i++)
	{
		if (attr.name[i] == cur_attr_name)
		{
			index = i;
			break;
		}
	}
	//if not found
	if (index == -1)
	{
		std::cout << "cur_attr_name not found in record manager when generating index\n";
		throw attribute_not_exist();
	}
	//get block number
	int block_num = getBlockNumber(table_name);
	//get file path
	std::string file_path = "INDEX_FILE_" + cur_attr_name + "_" + table_name;
	
	for (int i = 1; i < block_num; i++) {
		// 获得页指针
		char* page_pointer = buffer_manager.getPage(RECORD_PATH + table_name + ".db", i);
		int offset = 3 * sizeof(int);
		// 获得tuple个数
		int tuple_num = getTupleNum(page_pointer);
		for (int j = 0; j < tuple_num; j++) {
			std::vector<key_> data = decodeSingleTuple(page_pointer, offset).getKeys();
			index_manager.insert_index(file_path, data[index], i, data[index].type);
		}
	}
}

std::string RecordManager::encodeTuple(Tuple tuple) {
	std::stringstream s;
	int n = tuple.size();	//4bytes的Tuple size:n
	std::vector<key_> keys = tuple.getKeys();
	s.write((char*)&n, sizeof(int));
	for (int i = 0; i < n; i++) {
		s.write((char*)&keys[i].type, sizeof(int));		//写入key_tupe
		s.write((char*)&(keys[i].INT_VALUE), sizeof(int));	//写入INT
		s.write((char*)&(keys[i].FLOAT_VALUE), sizeof(float));	//写入FLOAT
		int string_len = keys[i].STRING_VALUE.size();
		s.write((char*)&string_len, sizeof(int));	//写入4 bytes sizeof(STRING_VALUE)
		s.write(keys[i].STRING_VALUE.c_str(), string_len);	//写入STRING_VALUE
	}
	return s.str();
}

Tuple RecordManager::decodeSingleTuple(char* pointer, int& offset) {
	Tuple tuple;
	int n;
	memcpy(&n,  pointer + offset, sizeof(int));	//读取4bytes的Tuple size:n
	offset += sizeof(int);
	for (int i = 0; i < n; i++) {
		key_ key;
		memcpy(&(key.type),  pointer + offset, sizeof(int)); //读取key_tupe
		offset += sizeof(int);
		memcpy(&(key.INT_VALUE),pointer + offset, sizeof(int)); //读取INT
		offset += sizeof(int);
		memcpy(&(key.FLOAT_VALUE),  pointer + offset, sizeof(float)); //读取float
		offset += sizeof(float);
		int string_len;
		memcpy(&string_len,  pointer + offset, sizeof(int));
		offset += sizeof(int);
		char* value = (char*)malloc(string_len + 1);
		memcpy(value,  pointer + offset, string_len);
		value[string_len] = '\0';
		offset += string_len;
		key.STRING_VALUE = value;
		free(value);
		tuple.addKey(key);
	}
	return tuple;
}

std::vector<Tuple> RecordManager::decodeAllTuple(char* pointer) {
	std::vector<Tuple> allTuples;
	int offset = 3 * sizeof(int);
	int tuple_num = getTupleNum(pointer);
	for (int i = 0; i < tuple_num; i++) {
		allTuples.push_back(decodeSingleTuple(pointer, offset));
	}
	return allTuples;
}

int RecordManager::getBlockNumber(std::string table_name) {
	std::string filepath = RECORD_PATH + table_name + ".db";
	int block_id = 0;
	char* first_page = buffer_manager.getPage(filepath, 0);
	memcpy(&block_id, first_page + sizeof(int), sizeof(int));
    return block_id;
}

void RecordManager::setBlockNumber(int block_number, std::string table_name)
{
	std::string filepath = RECORD_PATH + table_name + ".db";
	char* first_page = buffer_manager.getPage(filepath, 0);
	memcpy(first_page + sizeof(int), &block_number , sizeof(int));
	buffer_manager.setDirty(buffer_manager.getPageId(filepath, 0));
}

std::string RecordManager::createFormatedBlockString()
{
	std::stringstream s;
	char init[sizeof(int) + 1] = "####";
	int size = 12;	//一块block的前4个bytes存block总占用空间
	int TupleNumber = 0;	//后4个bytes存block有多少个Tuple，随后Tuple按顺序密集存放
	s.write(init, sizeof(int));
	s.write((char*)&size, sizeof(int));
	s.write((char*)&TupleNumber, sizeof(int));
	return s.str();
}

int RecordManager::getBlockStringSize(char* pointer)
{
	int size = 0;
	pointer += sizeof(int);
	memcpy(&size, pointer, sizeof(int));
	return size;
}

void RecordManager::setBlockStringSize(int size, char* pointer)
{
	pointer += sizeof(int);
	memcpy(pointer,  &size, sizeof(int));
}

int RecordManager::getTupleNum(char* pointer)
{
	int size = 0;
	pointer += sizeof(int) * 2;
	memcpy(&size,pointer, sizeof(int));
	return size;
}

void RecordManager::setTupleStringSize(int size, char* pointer)
{
	pointer += sizeof(int) * 2;
	memcpy(pointer, &size, sizeof(int));
}

void RecordManager::writeTupleString(char* pointer, std::string encoded_tuple)
{
	int old_size = getBlockStringSize(pointer);
	char* end = pointer + old_size;
	for (size_t i = 0; i < encoded_tuple.size(); i++) end[i] = encoded_tuple[i];	//写入新的tuple
	setBlockStringSize(old_size + encoded_tuple.size(), pointer);
	setTupleStringSize(getTupleNum(pointer) + 1, pointer);	//设置BlockSize和Tuple个数
}

bool RecordManager::meetRelation(key_ key, Relation relation)
{
	bool meetRelation = false;
	//先判断tuple的类型
	if (relation.key.type == INT) {
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
	else if (relation.key.type == FLOAT) {
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
	else {
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

bool RecordManager::haveSameKey(Table& table, int i, key_ key) {
	// 以下方法为：加载整张表→从头遍历所有的数据，判断重复
	// 表中所有的record
	std::vector<Tuple>& tuple = table.getTuple();
	for (size_t m = 0; m < tuple.size(); m++) {
		if (key.type == INT) {
			if (tuple[m].getKeys()[i].INT_VALUE == key.INT_VALUE)
				return true;
		}
		else if (key.type == FLOAT) {
			if (tuple[m].getKeys()[i].FLOAT_VALUE == key.FLOAT_VALUE)
				return true;
		}
		else {
			if (tuple[m].getKeys()[i].STRING_VALUE == key.STRING_VALUE)
				return true;
		}
	}
	return false;
}
