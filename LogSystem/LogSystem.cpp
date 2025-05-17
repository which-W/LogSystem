// LogSystem.cpp: 定义应用程序的入口点。
//

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
