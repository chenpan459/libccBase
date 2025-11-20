# 日志模块

add_executable(logger_test
    test/logger_test.cc
)

target_link_libraries(logger_test PRIVATE pthread)

