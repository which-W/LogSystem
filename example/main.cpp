#include "Logger.h"
int main() {
    logsystem::Logger logger({"example", "example.jsonl", logsystem::LogLevel::Debug, true});
    logger.write({logsystem::LogLevel::Info, "application.started", "example started", {{"port", std::int64_t(8080)}}});
    logger.log(logsystem::LogLevel::Info, "formatted {}", "message");
}
