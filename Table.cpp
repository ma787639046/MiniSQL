#include "Table.h"

bool Table::setIndex(short index,std::string index_name) {
    //判断原Index是否存在序号相同或同名情况
	for (int i = 0; i < this->index.indexNumber; i++) {
		if (index == this->index.location[i]) {
			std::cout << "Index设置失败，因为这个属性已被设置了Index" << std::endl;
			return false;
		}
	}
	for (int i = 0; i < this->index.indexNumber; i++) {
		if (index_name == this->index.indexname[i]) {
			std::cout << "无法设置Index：Index重复" << std::endl;
			return false;
		}
	}
	//无重复index或重名，开始添加index
	this->index.location[this->index.indexNumber] = index;
	this->index.indexname[this->index.indexNumber] = index_name;
	this->index.indexNumber++;
	return true;	//返回true，设置成功
}

bool Table::dropIndex(std::string index_name)
{
	//遍历indexname，获得index_name的位置
	for (int i = 0; i < this->index.indexNumber; i++) {
		if (this->index.indexname[i] == index_name) {
            //找到对应的index_name，将末尾元素赋给这个index，pop末尾元素，以删除这个index
            index.location[i] = index.location[this->index.indexNumber - 1];
            index.indexname[i] = index.indexname[this->index.indexNumber - 1];
            this->index.indexNumber--;
            return true;	//返回true，删除成功
        }
	}
	//没有找到对应的index_name，报错
    std::cout << "无法删除Index：没有找到名为 " << index_name << " 的Index" << std::endl;
    return false;
}

void Table::showTable()
{
	for (int i = 0; i < attribute.num; i++) {
		std::cout << attribute.name[i] << "\t";
	}
	std::cout << std::endl;
	for (int i = 0; i < tuple.size(); i++) {
		tuple[i].showTuple();
	}
}

void Table::showTable(int limit)
{
	for (int i = 0; i < attribute.num; i++) {
		std::cout << attribute.name[i] << "\t";
	}
	std::cout << std::endl;
	for (int i = 0; i < tuple.size() && i < limit; i++) {
		tuple[i].showTuple();
	}
}
	