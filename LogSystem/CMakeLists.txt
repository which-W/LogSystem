add_library(LogSystem STATIC Logger.cpp LogQueue.cpp)
add_library(LogSystem::LogSystem ALIAS LogSystem)
target_include_directories(LogSystem PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/logsystem>)
target_compile_features(LogSystem PUBLIC cxx_std_17)
install(TARGETS LogSystem EXPORT LogSystemTargets ARCHIVE DESTINATION lib)
install(FILES Logger.h LogQueue.h global.h DESTINATION include/logsystem)
install(EXPORT LogSystemTargets NAMESPACE LogSystem:: DESTINATION lib/cmake/LogSystem)
install(FILES "${PROJECT_SOURCE_DIR}/LICENSE" DESTINATION share/licenses/LogSystem)
