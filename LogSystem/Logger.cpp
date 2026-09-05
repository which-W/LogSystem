#include "Logger.h"
#include <algorithm>
#include <chrono>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif
namespace logsystem { namespace {
std::string jsonEscape(const std::string& input) {
    std::ostringstream output;
    for (const unsigned char ch : input) {
        switch (ch) {
        case '"': output << "\\\""; break; case '\\': output << "\\\\"; break;
        case '\b': output << "\\b"; break; case '\f': output << "\\f"; break;
        case '\n': output << "\\n"; break; case '\r': output << "\\r"; break;
        case '\t': output << "\\t"; break;
        default:
            if (ch < 0x20) output << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(ch) << std::dec;
            else output << static_cast<char>(ch);
        }
    }
    return output.str();
}
std::string timestamp() {
    const auto now = std::chrono::system_clock::now();
    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    const std::time_t time = std::chrono::system_clock::to_time_t(now); std::tm utc {};
#ifdef _WIN32
    gmtime_s(&utc, &time);
#else
    gmtime_r(&time, &utc);
#endif
    std::ostringstream output;
    output << std::put_time(&utc, "%Y-%m-%dT%H:%M:%S") << '.' << std::setw(3) << std::setfill('0') << millis.count() << 'Z';
    return output.str();
}
std::uint64_t processId() {
#ifdef _WIN32
    return static_cast<std::uint64_t>(_getpid());
#else
    return static_cast<std::uint64_t>(getpid());
#endif
}
void appendValue(std::ostringstream& output, const LogValue& value) {
    std::visit([&output](const auto& item) {
        using T = std::decay_t<decltype(item)>;
        if constexpr (std::is_same_v<T, std::string>) output << '"' << jsonEscape(item) << '"';
        else if constexpr (std::is_same_v<T, bool>) output << (item ? "true" : "false");
        else output << item;
    }, value);
}
int levelRank(LogLevel level) {
    switch (level) { case LogLevel::Debug: return 0; case LogLevel::Info: return 1; case LogLevel::Warn: return 2; case LogLevel::Error: return 3; }
    return 3;
}
} // namespace
const char* toString(LogLevel level) noexcept {
    switch (level) { case LogLevel::Debug: return "DEBUG"; case LogLevel::Info: return "INFO"; case LogLevel::Warn: return "WARN"; case LogLevel::Error: return "ERROR"; }
    return "ERROR";
}
LogLevel parseLevel(const std::string& value) {
    std::string normalized = value;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    if (normalized == "debug") return LogLevel::Debug;
    if (normalized == "info") return LogLevel::Info;
    if (normalized == "warn" || normalized == "warning") return LogLevel::Warn;
    if (normalized == "error") return LogLevel::Error;
    throw std::invalid_argument("invalid log level: " + value);
}
Logger::Logger(LoggerOptions options) : options_(std::move(options)), queue_(options_.queue_capacity) {
    if (!options_.console_output && !options_.file_output) throw std::invalid_argument("at least one log output must be enabled");
    if (options_.file_output) {
        if (options_.file_path.empty()) throw std::invalid_argument("log file path is empty");
        if (options_.max_file_size_bytes == 0 || options_.max_files == 0) throw std::invalid_argument("log rotation limits must be positive");
        openFile();
    }
    consumer_ = std::thread([this] { consume(); });
}
Logger::Logger(const std::string& filename, bool console_output, std::size_t max_file_size_kb, int)
    : Logger(LoggerOptions {"application", filename, LogLevel::Info, console_output, true, max_file_size_kb * 1024U, 5, 8192}) {}
Logger::~Logger() { shutdown(); }
bool Logger::enabled(LogLevel level) const noexcept { return levelRank(level) >= levelRank(options_.minimum_level); }
std::string Logger::serialize(const LogRecord& record) const {
    std::ostringstream output;
    output << "{\"timestamp\":\"" << timestamp() << "\",\"level\":\"" << toString(record.level)
           << "\",\"service\":\"" << jsonEscape(options_.service_name) << "\",\"pid\":" << processId()
           << ",\"thread\":\"" << jsonEscape(toString(std::this_thread::get_id())) << "\",\"event\":\""
           << jsonEscape(record.event) << "\",\"message\":\"" << jsonEscape(record.message) << '"';
    for (const auto& field : record.fields) {
        if (field.key.empty()) continue;
        output << ",\"" << jsonEscape(field.key) << "\":"; appendValue(output, field.value);
    }
    output << '}'; return output.str();
}
void Logger::write(LogRecord record) {
    if (stopped_.load() || !enabled(record.level)) return;
    auto line = serialize(record); pending_.fetch_add(1);
    const auto wait = levelRank(record.level) >= levelRank(LogLevel::Warn) ? std::chrono::milliseconds(50) : std::chrono::milliseconds(0);
    if (queue_.push(line, wait)) return;
    pending_.fetch_sub(1); dropped_.fetch_add(1); flushed_.notify_all();
    if (levelRank(record.level) >= levelRank(LogLevel::Warn)) { std::lock_guard<std::mutex> lock(output_mutex_); std::cerr << line << '\n'; }
}
void Logger::consume() {
    std::string line;
    while (queue_.pop(line)) { outputLine(line); pending_.fetch_sub(1); flushed_.notify_all(); }
    std::lock_guard<std::mutex> lock(output_mutex_); if (file_.is_open()) file_.flush();
}
void Logger::outputLine(const std::string& line) {
    std::lock_guard<std::mutex> lock(output_mutex_);
    if (options_.file_output) { rotateIfNeeded(line.size() + 1); file_ << line << '\n'; file_size_ += line.size() + 1; }
    if (options_.console_output) std::cout << line << '\n';
}
void Logger::openFile() {
    const auto parent = options_.file_path.parent_path(); if (!parent.empty()) std::filesystem::create_directories(parent);
    file_.open(options_.file_path, std::ios::out | std::ios::app | std::ios::binary);
    if (!file_) throw std::runtime_error("failed to open log file: " + options_.file_path.string());
    file_size_ = std::filesystem::exists(options_.file_path) ? static_cast<std::size_t>(std::filesystem::file_size(options_.file_path)) : 0;
}
void Logger::rotateIfNeeded(std::size_t incoming_bytes) {
    if (file_size_ + incoming_bytes <= options_.max_file_size_bytes) return;
    file_.close(); std::error_code ignored;
    std::filesystem::remove(options_.file_path.string() + '.' + std::to_string(options_.max_files), ignored);
    for (std::size_t index = options_.max_files; index > 1; --index) {
        const auto from = options_.file_path.string() + '.' + std::to_string(index - 1);
        const auto to = options_.file_path.string() + '.' + std::to_string(index);
        if (std::filesystem::exists(from)) std::filesystem::rename(from, to);
    }
    if (std::filesystem::exists(options_.file_path)) std::filesystem::rename(options_.file_path, options_.file_path.string() + ".1");
    openFile();
}
void Logger::flush() {
    std::unique_lock<std::mutex> lock(flush_mutex_); flushed_.wait(lock, [this] { return pending_.load() == 0; });
    std::lock_guard<std::mutex> output_lock(output_mutex_); if (file_.is_open()) file_.flush(); std::cout.flush();
}
void Logger::shutdown() {
    if (stopped_.exchange(true)) return; queue_.shutdown(); if (consumer_.joinable()) consumer_.join();
    std::lock_guard<std::mutex> lock(output_mutex_); if (file_.is_open()) file_.close();
}
} // namespace logsystem
