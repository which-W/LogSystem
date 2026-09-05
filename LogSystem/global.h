#pragma once
#include <sstream>
#include <string>
#include <utility>
namespace logsystem {
enum class LogLevel { Debug, Info, Warn, Error };
template <typename T> std::string toString(T&& value) {
    std::ostringstream output; output << std::forward<T>(value); return output.str();
}
} // namespace logsystem
