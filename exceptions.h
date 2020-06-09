//
//	exceptions.h
//	Created by MGY on 2020/06/05
//

/*
	exceptions.h 定义了异常类。
	异常通常由底层模块产生，例如Catalog Manager查找表时，表不存在，会抛出table_not_exist异常
	在API层之上的顶层模块接收到(catch)异常后，根据异常的种类，反馈给用户提示信息
	例如：
	try {
		……
	}
	catch (table_name_conflict e) {
		std::cout << "Select Fail. No such table name" << tablename << std::endl;
	}
*/
#pragma once

#include <exception>

class table_name_conflict : public std::exception {};

class table_not_exist : public std::exception {};

class attribute_not_exist : public std::exception {};

class attribute_not_unique :public std::exception {};

class index_name_conflict : public std::exception {};

class index_exist_conflict : public std::exception {};

class index_not_exist : public std::exception {};

class index_full : public std::exception {};

class tuple_type_conflict : public std::exception {};

class tuple_out_of_range : public std::exception {};	//decode化的tuple超过PAGESIZE

class primary_key_conflict : public std::exception {};

class key_type_conflict : public std::exception {};

class input_format_error : public std::exception {};

class exit_command : public std::exception {};

class unique_conflict :public std::exception {};