#include "RecordManager.h"

int main() {
	RecordManager m;
	Tuple tuple;
	key_ key;
	key.INT_VALUE = 0;
	tuple.addTuple(key);
	key.FLOAT_VALUE = 2.1f;
	tuple.addTuple(key);
	key.STRING_VALUE = "ABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGH";
	tuple.addTuple(key);

	return 0;
}

void RecordManager::createTableFile(std::string table_name)
{
	table_name = "./RecordFiles/" + table_name + ".db";
	FILE* f = fopen(table_name.c_str(), "w");
	fclose(f);
	std::string empty_block = createFormatedBlockString();
	flushToBlock(empty_block, 0, table_name);	//将FormatedBlock刷入文件头
}

void RecordManager::dropTableFile(std::string table_name)
{
	table_name = "./RecordFiles/" + table_name + ".db";
	remove(table_name.c_str());
}

std::vector<Tuple> RecordManager::loadTuples(std::string table_name)
{
	// 获取总block数量
	BufferManager* buffer_manager = new BufferManager(table_name, 1);
	int block_number = buffer_manager->getBlockNum();
	delete(buffer_manager);
	// 从第一个block开始，装入Tuple
	std::vector<Tuple> tuple;
	for (int i = 0; i < block_number; i++) {
		std::string s = loadBlockString(i, table_name);	//装入block
		std::vector<Tuple> temp = readTupleString(s);	//decode blockString为vector<Tuple>
		for (int j = 0; j < temp.size(); j++)
			tuple.push_back(temp[i]);
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

int RecordManager::getTupleStringSize(std::string s)
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
	std::string filename = "./RecordFiles/" + table_name + ".db";
	BufferManager buffer(filename, 1);	//因为只需要刷一个block，因此需要1个page
	int page_id = buffer.loadBlock(block_id);	//装入block
	char* pointer = buffer.getPagePointer(page_id);	//得到page pointer
	//写入page
	for (int i = 0; i < s.size(); i++)
		pointer[i] = s[i];
	buffer.setDirty(page_id, true);	//设置脏页
}

std::string RecordManager::loadBlockString(int block_id, std::string table_name)
{
	std::string filename = "./RecordFiles/" + table_name + ".db";
	BufferManager buffer(filename, 1);	//因为只需要装一个block，因此需要1个page
	int page_id = buffer.loadBlock(block_id);	//装入block
	char* pointer = buffer.getPagePointer(page_id);	//得到page pointer
	std::string block;
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
	int tuple_number = getTupleStringSize(s) + 1;
	setBlockStringSize(size, s);
	setTupleStringSize(tuple_number, s);	//设置BlockSize和Tuple个数
}

std::vector<Tuple> RecordManager::readTupleString(std::string& s)
{
	std::vector<Tuple> tuples;
	int tupleNum = getTupleStringSize(s);
	std::string s_temp = s.substr(2 * sizeof(int));
	
	for (int i = 0; i < tupleNum; i++) {
		Tuple* t = new(Tuple);
		(*t).decodeTuple(s_temp);
		tuples.push_back(*t);
		delete(t);
	}
	return tuples;
}
