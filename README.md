# LogSystem

A C++17 asynchronous JSON Lines logging library. It uses a bounded multi-producer,
single-consumer queue so file writes and rotation are serialized.

## Build

```sh
cmake -S . -B build -DLOGSYSTEM_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Consumers can use `add_subdirectory` and link `LogSystem::LogSystem`:

```cmake
add_subdirectory(third_party/LogSystem)
target_link_libraries(my_service PRIVATE LogSystem::LogSystem)
```

## API

```cpp
#include "Logger.h"

logsystem::LoggerOptions options;
options.service_name = "gate_server";
options.file_path = "logs/gate_server.jsonl";
options.console_output = true;

logsystem::Logger logger(options);
logger.write({logsystem::LogLevel::Info, "server.started", "server is ready",
              {{"port", std::int64_t(8080)}}});
logger.log(logsystem::LogLevel::Info, "legacy formatted call: {}", 42);
logger.flush();
```

`LoggerOptions` defaults to INFO, a 50 MiB rotation threshold, five retained files,
and an 8192-entry queue. DEBUG/INFO records are dropped and counted when the queue
is full. WARN/ERROR records wait up to 50 ms and then fall back to `stderr`.
`shutdown()` and the destructor drain accepted records before returning.

Every output line is a standalone JSON object with UTC timestamp, severity, service,
process, thread, event, message, and caller-provided typed fields. Avoid placing
passwords, tokens, verification codes, or user content in log fields.

The original constructor treats its size argument as KiB for source compatibility:

```cpp
Logger logger("application.jsonl", true, 100); // 100 KiB
```

## License

Apache-2.0. Installation places the license under `share/licenses/LogSystem`.
