# LogSystem

### 一、概述

这是一个基于C++的异步日志系统，采用多生产者单消费者模型，具备日志文件过大自动划分功能，同时支持文件输出和终端输出两种方式。本次更新添加了多消费者机制，实现了多线程并发处理日志队列消息。

### 二、技术原理及实现细节

#### 1. 多生产者单消费者模型（基础）

在该模型中，多个生产者线程负责生成日志信息并放入共享的数据缓冲区，单个消费者线程从缓冲区中取出日志进行处理。生产者之间需维持互斥关系，避免数据混乱；生产者与消费者之间存在同步和互斥关系，确保缓冲区合理使用。

以下是一个该日志系统简单的使用示例代码：

```cpp
#include "LogSystem.h"
#include "global.h"
#include "Logger.h"
using namespace std;

int main()
{
	Logger  logger("log.txt", true, 100);
	try {
		
		logger.log(LogLevel::INFO, "starting func {}", "server");
		logger.log(LogLevel::INFO, "starting func {}", "server");
		logger.log(LogLevel::INFO, "starting func {}", "server");
		logger.log(LogLevel::INFO, "starting func {}", "server");
		int user_id = 42;
		std::string action = "login";
		double duration = 3.5;
		std::string world = "World";
		logger.log(LogLevel::INFO, "User {} performed {} in {} seconds.", user_id, action, duration);
		logger.log(LogLevel::INFO, "User {} performed {} in {} seconds.", user_id, action, duration);
		logger.log(LogLevel::INFO, "User {} performed {} in {} seconds.", user_id, action, duration);
		logger.log(LogLevel::INFO, "User {} performed {} in {} seconds.", user_id, action, duration);
		logger.log(LogLevel::INFO, "Hello {}", world);
		logger.log(LogLevel::INFO, "This is a message without placeholders.");
		logger.log(LogLevel::INFO, "Multiple placeholders: {}, {}, {}.", 1, 2, 3);
			

		}
	catch (std::exception& ec) {
		cout << ec.what() << endl;
	}
	return 0;
}
```
#### 2.使用多线程处理消息队列（拓展）

