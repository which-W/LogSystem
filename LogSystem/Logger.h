#pragma once
#include "LogQueue.h"
#include "global.h"
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>
#include <variant>
#include <vector>
namespace logsystem {
using LogValue = std::variant<std::string, std::int64_t, std::uint64_t, double, bool>;
struct LogField { std::string key; LogValue value; };
struct LogRecord {
    LogLevel level = LogLevel::Info;
    std::string event;
    std::string message;
    std::vector<LogField> fields;
};
struct LoggerOptions {
    std::string service_name = "application";
    std::filesystem::path file_path;
    LogLevel minimum_level = LogLevel::Info;
    bool console_output = false;
    bool file_output = true;
    std::size_t max_file_size_bytes = 50U * 1024U * 1024U;
    std::size_t max_files = 5;
    std::size_t queue_capacity = 8192;
};
class Logger {
public:
    explicit Logger(LoggerOptions options);
    Logger(const std::string& filename, bool console_output = false,
           std::size_t max_file_size_kb = 100, int ignored_num_threads = 1);
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;
    void write(LogRecord record);
    template <typename... Args> void log(LogLevel level, const std::string& format, Args&&... args) {
        write({level, "log", formatMessage(format, std::forward<Args>(args)...), {}});
    }
    void flush();
    void shutdown();
    std::uint64_t droppedCount() const noexcept { return dropped_.load(); }
private:
    template <typename... Args> static std::string formatMessage(const std::string& format, Args&&... args) {
        const std::vector<std::string> values {toString(std::forward<Args>(args))...};
        std::ostringstream output; std::size_t cursor = 0, value = 0;
        while (true) {
            const auto placeholder = format.find("{}", cursor);
            if (placeholder == std::string::npos) break;
            output << format.substr(cursor, placeholder - cursor)
                   << (value < values.size() ? values[value++] : "{}");
            cursor = placeholder + 2;
        }
        output << format.substr(cursor);
        while (value < values.size()) output << values[value++];
        return output.str();
    }
    bool enabled(LogLevel level) const noexcept;
    std::string serialize(const LogRecord& record) const;
    void consume();
    void outputLine(const std::string& line);
    void openFile();
    void rotateIfNeeded(std::size_t incoming_bytes);
    LoggerOptions options_;
    LogQueue queue_;
    std::thread consumer_;
    std::ofstream file_;
    std::mutex output_mutex_, flush_mutex_;
    std::condition_variable flushed_;
    std::atomic<std::uint64_t> pending_ {0}, dropped_ {0};
    std::atomic<bool> stopped_ {false};
    std::size_t file_size_ = 0;
};
const char* toString(LogLevel level) noexcept;
LogLevel parseLevel(const std::string& value);
} // namespace logsystem
using Logger = logsystem::Logger;
using LogLevel = logsystem::LogLevel;
