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
	Logger(const std::string& filename, bool console_output = false, size_t max_file_size_kb = 100, int num_threads = 3);
	~Logger();
	template<typename ...Args>
	void log(LogLevel Loglevel, const std::string& format, Args&& ...args) {
		std::lock_guard<std::mutex>lock(_mutex); // 锁定互斥量，确保线程安全
		std::string loglevel_str;
		switch (Loglevel) {
		case LogLevel::INFO: loglevel_str = "[INFO] "; break;
		case LogLevel::DEBUG: loglevel_str = "[DEBUG] "; break;
		case LogLevel::ERROR: loglevel_str = "[ERROR] "; break;
		}
		_log_queue.push(loglevel_str + formatMessage(format, std::forward<Args>(args)...));
	}

	template<typename... Args>
	void console_log(LogLevel level, const std::string& format, Args&&... args) {
		std::lock_guard<std::mutex>lock(_mutex); // 锁定互斥量，确保线程安全
		std::string loglevel_str;
		switch (level) {
		case INFO: loglevel_str = "[INFO] "; break;
		case DEBUG: loglevel_str = "[DEBUG] "; break;
		case WARN: loglevel_str = "[WARN] "; break;
		case ERROR: loglevel_str = "[ERROR] "; break;
		}
		std::cout << (loglevel_str + formatMessage(format, std::forward<Args>(args)...)) << std::endl;
	}



private:
	void open_log_file();

	void check_and_rotate_log_file();

	size_t get_file_size();

	void rotate_log_file();

	//时间戳
	std::string get_time();
	// 使用模板折叠格式化日志消息，支持 "{}" 占位符
	template<typename... Args>
	std::string formatMessage(const std::string& format, Args&&... args) {
		std::vector<std::string> arg_strings = { to_string_helper(std::forward<Args>(args))... };
		std::ostringstream oss;
		size_t arg_index = 0;
		size_t pos = 0;
		size_t placeholder = format.find("{}", pos);

		while (placeholder != std::string::npos) {
			oss << format.substr(pos, placeholder - pos);
			if (arg_index < arg_strings.size()) {
				oss << arg_strings[arg_index++];
			}
			else {
				// 没有足够的参数，保留 "{}"
				oss << "{}";
			}
			pos = placeholder + 2; // 跳过 "{}"
			placeholder = format.find("{}", pos);
		}

		// 添加剩余的字符串
		oss << format.substr(pos);

		// 如果还有剩余的参数，按原方式拼接
		while (arg_index < arg_strings.size()) {
			oss << arg_strings[arg_index++];
		}

		return "[" + get_time() + "] " + oss.str();
	}

	

	std::vector<std::thread> _work_threads;
	std::ofstream _log_file;
	std::filesystem::path _log_dir;
	std::atomic<bool> _exit_flag;
	LogQueue _log_queue;
	bool _console_output; // 控制是否输出到控制台
	size_t _max_file_size;
	std::string _base_filename;
	int _current_log_index = 0;
	std::mutex _mutex;
	int _num_threads;
};

