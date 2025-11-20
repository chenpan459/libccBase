#ifndef __LOGGER__
#define __LOGGER__

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace cLogger {

/**
 * @brief 日志级别
 */
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    FATAL = 4
};

/**
 * @brief 日志配置
 */
struct LoggerConfig {
    LogLevel min_level = LogLevel::DEBUG;  // 最小日志级别
    bool enable_console = true;             // 是否输出到控制台
    bool enable_file = false;               // 是否输出到文件
    std::string log_file_path = "app.log"; // 日志文件路径
    bool enable_thread_id = true;          // 是否显示线程ID
    bool enable_file_info = true;          // 是否显示文件信息（文件名、行号、函数名）
    size_t max_file_size = 10 * 1024 * 1024;  // 最大文件大小（10MB）
    int max_backup_files = 5;              // 最大备份文件数
    bool async_mode = false;                // 是否使用异步模式
};

/**
 * @brief 线程安全的日志类
 */
class Logger {
   public:
    /**
     * @brief 获取单例实例
     */
    static Logger& GetInstance() {
        static Logger instance;
        return instance;
    }

    /**
     * @brief 初始化日志系统
     * @param config 日志配置
     */
    void Initialize(const LoggerConfig& config) {
        std::lock_guard<std::mutex> lock(mutex_);
        config_ = config;

        if (config_.enable_file) {
            OpenLogFile();
        }

        if (config_.async_mode) {
            StartAsyncWorker();
        }
    }

    /**
     * @brief 设置日志级别
     */
    void SetLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(mutex_);
        config_.min_level = level;
    }

    /**
     * @brief 记录日志
     * @param level 日志级别
     * @param file 文件名
     * @param line 行号
     * @param function 函数名
     * @param message 日志消息
     */
    void Log(LogLevel level, const char* file, int line, const char* function,
             const std::string& message) {
        if (level < config_.min_level) {
            return;
        }

        std::string log_entry = FormatLog(level, file, line, function, message);

        if (config_.async_mode) {
            // 异步模式：将日志放入队列
            std::lock_guard<std::mutex> lock(queue_mutex_);
            log_queue_.push(log_entry);
            queue_cv_.notify_one();
        } else {
            // 同步模式：直接输出
            WriteLog(log_entry);
        }
    }

    /**
     * @brief 刷新日志缓冲区
     */
    void Flush() {
        if (config_.async_mode) {
            // 等待队列为空
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] { return log_queue_.empty(); });
        }

        std::lock_guard<std::mutex> lock(mutex_);
        if (log_file_.is_open()) {
            log_file_.flush();
        }
        std::cout.flush();
    }

    /**
     * @brief 关闭日志系统
     */
    void Shutdown() {
        if (config_.async_mode) {
            StopAsyncWorker();
        }

        std::lock_guard<std::mutex> lock(mutex_);
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }

   private:
    Logger() = default;
    ~Logger() { Shutdown(); }
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void OpenLogFile() {
        if (log_file_.is_open()) {
            log_file_.close();
        }
        log_file_.open(config_.log_file_path, std::ios::app);
        if (!log_file_.is_open()) {
            std::cerr << "Failed to open log file: " << config_.log_file_path
                      << std::endl;
        }
    }

    std::string FormatLog(LogLevel level, const char* file, int line,
                          const char* function, const std::string& message) {
        std::ostringstream oss;

        // 时间戳
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) %
                  1000;

        char time_str[64];
        std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S",
                      std::localtime(&time_t));
        oss << "[" << time_str << "." << std::setfill('0') << std::setw(3)
            << ms.count() << "]";

        // 日志级别
        oss << " [" << LevelToString(level) << "]";

        // 线程ID
        if (config_.enable_thread_id) {
            oss << " [T:" << std::this_thread::get_id() << "]";
        }

        // 文件信息
        if (config_.enable_file_info) {
            std::string filename = ExtractFileName(file);
            oss << " [" << filename << ":" << line << ":" << function << "]";
        }

        // 日志消息
        oss << " " << message << std::endl;

        return oss.str();
    }

    std::string LevelToString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG:
                return "DEBUG";
            case LogLevel::INFO:
                return "INFO";
            case LogLevel::WARN:
                return "WARN";
            case LogLevel::ERROR:
                return "ERROR";
            case LogLevel::FATAL:
                return "FATAL";
            default:
                return "UNKNOWN";
        }
    }

    std::string ExtractFileName(const char* filepath) {
        std::string path(filepath);
        size_t pos = path.find_last_of("/\\");
        if (pos != std::string::npos) {
            return path.substr(pos + 1);
        }
        return path;
    }

    void WriteLog(const std::string& log_entry) {
        std::lock_guard<std::mutex> lock(mutex_);

        // 输出到控制台
        if (config_.enable_console) {
            std::cout << log_entry;
        }

        // 输出到文件
        if (config_.enable_file && log_file_.is_open()) {
            log_file_ << log_entry;
            log_file_.flush();

            // 检查文件大小，进行日志轮转
            CheckAndRotateLog();
        }
    }

    void CheckAndRotateLog() {
        if (!log_file_.is_open()) {
            return;
        }

        // 获取当前文件大小
        log_file_.seekp(0, std::ios::end);
        size_t file_size = log_file_.tellp();
        log_file_.seekp(0, std::ios::end);

        if (file_size >= config_.max_file_size) {
            log_file_.close();

            // 轮转日志文件
            for (int i = config_.max_backup_files - 1; i > 0; --i) {
                std::string old_file = config_.log_file_path + "." +
                                       std::to_string(i);
                std::string new_file = config_.log_file_path + "." +
                                       std::to_string(i + 1);
                std::ifstream src(old_file, std::ios::binary);
                if (src.is_open()) {
                    std::ofstream dst(new_file, std::ios::binary);
                    dst << src.rdbuf();
                    src.close();
                    dst.close();
                }
            }

            // 将当前日志文件重命名为 .1
            std::string backup_file = config_.log_file_path + ".1";
            std::ifstream src(config_.log_file_path, std::ios::binary);
            if (src.is_open()) {
                std::ofstream dst(backup_file, std::ios::binary);
                dst << src.rdbuf();
                src.close();
                dst.close();
            }

            // 重新打开日志文件
            OpenLogFile();
        }
    }

    void StartAsyncWorker() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (async_running_) {
            return;
        }
        async_running_ = true;
        async_thread_ = std::thread(&Logger::AsyncWorkerLoop, this);
    }

    void StopAsyncWorker() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!async_running_) {
                return;
            }
            async_running_ = false;
        }
        queue_cv_.notify_all();
        if (async_thread_.joinable()) {
            async_thread_.join();
        }

        // 处理剩余的日志
        std::lock_guard<std::mutex> lock(queue_mutex_);
        while (!log_queue_.empty()) {
            WriteLog(log_queue_.front());
            log_queue_.pop();
        }
    }

    void AsyncWorkerLoop() {
        while (true) {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock,
                           [this] { return !log_queue_.empty() || !async_running_; });

            if (!async_running_ && log_queue_.empty()) {
                break;
            }

            while (!log_queue_.empty()) {
                std::string log_entry = log_queue_.front();
                log_queue_.pop();
                lock.unlock();
                WriteLog(log_entry);
                lock.lock();
            }
        }
    }

    mutable std::mutex mutex_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    LoggerConfig config_;
    std::ofstream log_file_;
    std::queue<std::string> log_queue_;
    std::thread async_thread_;
    std::atomic<bool> async_running_{false};
};

// 便捷宏定义
#define LOG_DEBUG(message) \
    cLogger::Logger::GetInstance().Log( \
        cLogger::LogLevel::DEBUG, __FILE__, __LINE__, __FUNCTION__, message)

#define LOG_INFO(message) \
    cLogger::Logger::GetInstance().Log( \
        cLogger::LogLevel::INFO, __FILE__, __LINE__, __FUNCTION__, message)

#define LOG_WARN(message) \
    cLogger::Logger::GetInstance().Log( \
        cLogger::LogLevel::WARN, __FILE__, __LINE__, __FUNCTION__, message)

#define LOG_ERROR(message) \
    cLogger::Logger::GetInstance().Log( \
        cLogger::LogLevel::ERROR, __FILE__, __LINE__, __FUNCTION__, message)

#define LOG_FATAL(message) \
    cLogger::Logger::GetInstance().Log( \
        cLogger::LogLevel::FATAL, __FILE__, __LINE__, __FUNCTION__, message)

}  // namespace cLogger

#endif  // __LOGGER__

