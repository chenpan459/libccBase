#include <iostream>
#include <thread>
#include <vector>
#include "logger.h"

using namespace cLogger;
using std::cout;
using std::endl;

void TestBasicLogging() {
    cout << "\n========== 测试1: 基本日志功能 ==========" << endl;

    LoggerConfig config;
    config.min_level = LogLevel::DEBUG;
    config.enable_console = true;
    config.enable_file = false;
    config.enable_thread_id = true;
    config.enable_file_info = true;

    Logger::GetInstance().Initialize(config);

    LOG_DEBUG("这是一条调试日志");
    LOG_INFO("这是一条信息日志");
    LOG_WARN("这是一条警告日志");
    LOG_ERROR("这是一条错误日志");
    LOG_FATAL("这是一条致命错误日志");

    Logger::GetInstance().Flush();
}

void TestLogLevel() {
    cout << "\n========== 测试2: 日志级别过滤 ==========" << endl;

    LoggerConfig config;
    config.min_level = LogLevel::WARN;  // 只显示 WARN 及以上级别
    config.enable_console = true;
    config.enable_file = false;

    Logger::GetInstance().Initialize(config);

    LOG_DEBUG("这条DEBUG日志不会显示");
    LOG_INFO("这条INFO日志不会显示");
    LOG_WARN("这条WARN日志会显示");
    LOG_ERROR("这条ERROR日志会显示");
    LOG_FATAL("这条FATAL日志会显示");

    Logger::GetInstance().Flush();
}

void TestFileLogging() {
    cout << "\n========== 测试3: 文件日志 ==========" << endl;

    LoggerConfig config;
    config.min_level = LogLevel::DEBUG;
    config.enable_console = true;
    config.enable_file = true;
    config.log_file_path = "test.log";
    config.max_file_size = 1024;  // 1KB，方便测试轮转
    config.max_backup_files = 3;

    Logger::GetInstance().Initialize(config);

    for (int i = 0; i < 100; ++i) {
        LOG_INFO("测试文件日志 " + std::to_string(i));
    }

    Logger::GetInstance().Flush();
    cout << "日志已写入文件: test.log" << endl;
}

void TestMultiThreadLogging() {
    cout << "\n========== 测试4: 多线程日志 ==========" << endl;

    LoggerConfig config;
    config.min_level = LogLevel::INFO;
    config.enable_console = true;
    config.enable_file = false;
    config.enable_thread_id = true;

    Logger::GetInstance().Initialize(config);

    std::vector<std::thread> threads;
    const int num_threads = 5;
    const int logs_per_thread = 10;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([i, logs_per_thread]() {
            for (int j = 0; j < logs_per_thread; ++j) {
                LOG_INFO("线程 " + std::to_string(i) + " 的第 " +
                         std::to_string(j) + " 条日志");
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    Logger::GetInstance().Flush();
    cout << "多线程日志测试完成，共 " << num_threads * logs_per_thread
         << " 条日志" << endl;
}

void TestAsyncLogging() {
    cout << "\n========== 测试5: 异步日志 ==========" << endl;

    LoggerConfig config;
    config.min_level = LogLevel::INFO;
    config.enable_console = true;
    config.enable_file = false;
    config.async_mode = true;  // 启用异步模式

    Logger::GetInstance().Initialize(config);

    // 快速写入大量日志
    for (int i = 0; i < 1000; ++i) {
        LOG_INFO("异步日志测试 " + std::to_string(i));
    }

    cout << "已提交 1000 条日志到队列" << endl;
    Logger::GetInstance().Flush();
    cout << "异步日志写入完成" << endl;
}

void TestFormatting() {
    cout << "\n========== 测试6: 日志格式化 ==========" << endl;

    LoggerConfig config;
    config.min_level = LogLevel::DEBUG;
    config.enable_console = true;
    config.enable_file = false;
    config.enable_thread_id = true;
    config.enable_file_info = true;

    Logger::GetInstance().Initialize(config);

    LOG_DEBUG("简单消息");
    LOG_INFO("带变量的消息: " + std::to_string(42));
    LOG_WARN("警告: 内存使用率 " + std::to_string(85) + "%");
    LOG_ERROR("错误代码: " + std::to_string(404));

    Logger::GetInstance().Flush();
}

void TestWithoutFileInfo() {
    cout << "\n========== 测试7: 不显示文件信息 ==========" << endl;

    LoggerConfig config;
    config.min_level = LogLevel::INFO;
    config.enable_console = true;
    config.enable_file = false;
    config.enable_thread_id = true;
    config.enable_file_info = false;  // 不显示文件信息

    Logger::GetInstance().Initialize(config);

    LOG_INFO("这条日志不显示文件信息");
    LOG_WARN("只显示时间、级别和消息");

    Logger::GetInstance().Flush();
}

int main() {
    cout << "========================================" << endl;
    cout << "    日志模块测试程序" << endl;
    cout << "========================================" << endl;

    try {
        TestBasicLogging();
        TestLogLevel();
        TestFileLogging();
        TestMultiThreadLogging();
        TestAsyncLogging();
        TestFormatting();
        TestWithoutFileInfo();

        cout << "\n========================================" << endl;
        cout << "    所有测试完成！" << endl;
        cout << "========================================" << endl;

        // 关闭日志系统
        Logger::GetInstance().Shutdown();
    } catch (const std::exception& e) {
        cout << "测试异常: " << e.what() << endl;
        return 1;
    }

    return 0;
}

