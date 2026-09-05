add_executable(logsystem_tests logger_test.cpp)
target_link_libraries(logsystem_tests PRIVATE LogSystem::LogSystem)
add_test(NAME logsystem_tests COMMAND logsystem_tests)
