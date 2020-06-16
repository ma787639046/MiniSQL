#include "Table.h"

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
	