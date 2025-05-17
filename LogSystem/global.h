#pragma once
#include<mutex>
#include<queue>
#include<string>
#include<condition_variable>
#include<chrono>
#include<fstream>
#include<atomic>
#include<sstream>
#include<vector>
#include<stdexcept>
#include<filesystem>

enum LogLevel
{
	INFO,
	DEBUG,
	WARN,
	ERROR,

};
//��������������������תΪΪ�ַ���
template<typename T>
std::string to_string_helper(T&& arg) {
	std::ostringstream oss;
	oss << std::forward<T>(arg);
	return oss.str();

}


