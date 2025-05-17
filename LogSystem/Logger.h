#pragma once
#include "global.h"
#include"LogQueue.h"
#include "Singgle.h"
class Logger 
{
public:
	Logger(const Logger&& other) = delete;
	Logger& operator = (const Logger&& other) = delete;
	Logger(const Logger& other) = delete;
	Logger& operator = (const Logger& other) = delete;
	Logger(const std::string& filename, bool console_output = false, size_t max_file_size_kb = 100);
	~Logger();
	template<typename ...Args>
	void log(LogLevel Loglevel, const std::string& format, Args&& ...args);
	template<typename... Args>
	void console_log(LogLevel level, const std::string& format, Args&&... args);



private:
	void open_log_file();

	void check_and_rotate_log_file();

	size_t get_file_size();

	void rotate_log_file();

	//时间戳
	std::string get_time();
	// 使用模板折叠格式化日志消息，支持 "{}" 占位符
	template<typename... Args>
	std::string formatMessage(const std::string& format, Args&&... args);

	

	std::thread _work_threads;
	std::ofstream _log_file;
	std::filesystem::path _log_dir;
	std::atomic<bool> _exit_flag;
	LogQueue _log_queue;
	bool _console_output; // 控制是否输出到控制台
	size_t _max_file_size;
	std::string _base_filename;
	int _current_log_index = 0;
};

