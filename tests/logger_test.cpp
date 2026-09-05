#include "Logger.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
namespace {
std::vector<std::string> lines(const std::filesystem::path& path) {
    std::ifstream input(path); std::vector<std::string> result; std::string line;
    while (std::getline(input, line)) result.push_back(line); return result;
}
bool require(bool condition, const char* message) { if (!condition) std::cerr << message << '\n'; return condition; }
}
int main() {
    const auto root = std::filesystem::temp_directory_path() / "logsystem-tests";
    std::filesystem::remove_all(root); std::filesystem::create_directories(root);
    try {
        const auto primary = root / "application.jsonl";
        {
            logsystem::LoggerOptions options; options.service_name = "test-service"; options.file_path = primary;
            options.minimum_level = logsystem::LogLevel::Info; options.max_file_size_bytes = 1024 * 1024;
            logsystem::Logger logger(options);
            logger.write({logsystem::LogLevel::Debug, "filtered", "never written", {}});
            logger.write({logsystem::LogLevel::Warn, "json.escape", "quote \" and newline\n", {{"ok", true}, {"count", std::int64_t(7)}}});
            logger.log(logsystem::LogLevel::Info, "hello {}", "world");
            std::vector<std::thread> producers;
            for (int thread = 0; thread < 4; ++thread) producers.emplace_back([&logger, thread] {
                for (int item = 0; item < 100; ++item)
                    logger.write({logsystem::LogLevel::Info, "concurrent", "record", {{"producer", std::int64_t(thread)}, {"item", std::int64_t(item)}}});
            });
            for (auto& producer : producers) producer.join(); logger.flush();
        }
        const auto records = lines(primary);
        if (!require(records.size() == 402, "filtered/concurrent/drain record count mismatch")) return 1;
        for (const auto& record : records) {
            if (!require(record.front() == '{' && record.back() == '}', "record is not JSON-shaped")
                || !require(record.find("\"service\":\"test-service\"") != std::string::npos, "service field missing")) return 1;
        }
        if (!require(records.front().find("quote \\\" and newline\\n") != std::string::npos, "JSON escaping failed")
            || !require(records.at(1).find("hello world") != std::string::npos, "legacy formatting failed")) return 1;
        const auto rotating = root / "rotate.jsonl";
        {
            logsystem::LoggerOptions options; options.file_path = rotating; options.max_file_size_bytes = 300; options.max_files = 2;
            logsystem::Logger logger(options);
            for (int i = 0; i < 30; ++i) logger.log(logsystem::LogLevel::Info, "rotation record {} xxxxxxxxxxxxxxxxxxxx", i);
        }
        if (!require(std::filesystem::exists(rotating) && std::filesystem::exists(rotating.string() + ".1"), "rotation failed")
            || !require(!std::filesystem::exists(rotating.string() + ".3"), "retention failed")) return 1;
        std::filesystem::remove_all(root); return 0;
    } catch (const std::exception& error) {
        std::filesystem::remove_all(root); std::cerr << error.what() << '\n'; return 1;
    }
}
