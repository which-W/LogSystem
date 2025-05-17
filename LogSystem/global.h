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
//辅助函数，将单个参数转为为字符串
template<typename T>
std::string to_string_helper(T&& arg) {
	std::ostringstream oss;
	oss << std::forward<T>(arg);
	return oss.str();

}


