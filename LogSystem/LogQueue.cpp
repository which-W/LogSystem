#include"LogQueue.h"



void LogQueue::push(const std::string& msg) {
	std::lock_guard<std::mutex>lock(_mutex);
	_queue.push(msg);
	_con_var.notify_one();

}

bool  LogQueue::pop(std::string& msg) {
	std::unique_lock<std::mutex>lock(_mutex);
	//返回false时就会挂起并解锁，返回true则会加锁
	_con_var.wait(lock, [this]() {
		return !_queue.empty() || is_shutdown;
		});
	//消费逻辑
	if (is_shutdown && _queue.empty()) {
		return false;
	}
	//主线程结束子线程继续处理
	while (is_shutdown && !_queue.empty()) {
		msg = _queue.front();
		_queue.pop();
		return false;
	}
	msg = _queue.front();
	_queue.pop();
	return true;

}


void  LogQueue::shutdown() {
	std::lock_guard<std::mutex> lock(_mutex);
	is_shutdown = true;
	_con_var.notify_all();
}