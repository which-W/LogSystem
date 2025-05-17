#pragma once
#include "global.h"

class LogQueue {
public:
	void push(const std::string& msg);

	bool pop(std::string& msg);

	void shutdown();
private:
	std::queue<std::string> _queue;
	std::mutex _mutex;
	std::condition_variable _con_var;
	bool is_shutdown = false;

};