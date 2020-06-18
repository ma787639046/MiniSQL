// 
//	main.cpp
//	Created by MGY on 2020/06/16
//

//	main is the toppest module that directly calls functions of Interpreter

#include <iostream>
#include "Interpreter.h"

BufferManager buffer_manager;

int main(int argc, char* argv[]) {
	std::cout << "====    Welcome to MiniSQL created by MGY & ZZJ!    ====\n";
	std::cout << "Please input your query:\n";
	std::string tmp_table_name = "";
	std::string cur_table_name = "";
	while (true)
	{
		Interpreter interpreter;
		if (cur_table_name != "") interpreter.set_cur_table_name(cur_table_name);
		interpreter.read_in_command();
		tmp_table_name = interpreter.catch_erro();
		if (tmp_table_name != "") cur_table_name = tmp_table_name;
	}
	return 0;
}
