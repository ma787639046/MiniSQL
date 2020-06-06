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

class primary_key_conflict : public std::exception {};

class key_type_conflict : public std::exception {};

class input_format_error : public std::exception {};

class exit_command : public std::exception {};

class unique_conflict :public std::exception {};