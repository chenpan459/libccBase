#ifndef __ASYNC_CALLBACK__
#define __ASYNC_CALLBACK__

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace cAsync {

/**
 * @brief 异步回调管理器
 * 支持异步执行回调函数，并可以等待所有回调完成
 */
class AsyncCallback {
   public:
    using CallbackFunc = std::function<void()>;

    AsyncCallback() : running_(false) {}
    ~AsyncCallback() { Stop(); }

    /**
     * @brief 启动异步回调处理线程
     */
    void Start() {
        std::lock_guard<std::mutex> lock(mutex_);
        StartUnlocked();
    }

    /**
     * @brief 停止异步回调处理
     */
    void Stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!running_) {
                return;
            }
            running_ = false;
        }
        cv_.notify_all();
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
    }

    /**
     * @brief 异步执行回调函数
     * @param callback 回调函数
     * @return future对象，可用于等待回调完成
     */
    std::future<void> Post(CallbackFunc callback) {
        auto promise = std::make_shared<std::promise<void>>();
        auto future = promise->get_future();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!running_) {
                StartUnlocked();
            }
            // 保存 future 以便 WaitAll() 可以等待
            pending_futures_.push_back(future.share());
            callbacks_.push([callback, promise]() {
                try {
                    callback();
                    promise->set_value();
                } catch (...) {
                    promise->set_exception(std::current_exception());
                }
            });
        }
        cv_.notify_one();
        return future;
    }

    /**
     * @brief 异步执行回调函数（带返回值）
     * @tparam T 返回值类型
     * @param callback 回调函数
     * @return future对象，可用于获取返回值
     */
    template <typename T>
    std::future<T> Post(std::function<T()> callback) {
        auto promise = std::make_shared<std::promise<T>>();
        auto future = promise->get_future();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!running_) {
                StartUnlocked();
            }
            // 将 future 转换为 void future 以便统一管理
            auto void_promise = std::make_shared<std::promise<void>>();
            auto void_future = void_promise->get_future();
            pending_futures_.push_back(void_future.share());
            callbacks_.push([callback, promise, void_promise]() {
                try {
                    promise->set_value(callback());
                    void_promise->set_value();
                } catch (...) {
                    auto exception_ptr = std::current_exception();
                    promise->set_exception(exception_ptr);
                    void_promise->set_exception(exception_ptr);
                }
            });
        }
        cv_.notify_one();
        return future;
    }

    /**
     * @brief 等待所有待处理的回调完成
     */
    void WaitAll() {
        std::vector<std::shared_future<void>> futures;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            futures = pending_futures_;
            pending_futures_.clear();
        }
        // 等待所有 future 完成
        for (auto& future : futures) {
            future.wait();
        }
    }

    /**
     * @brief 获取待处理的回调数量
     * @return 待处理的回调数量
     */
    size_t GetPendingCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return callbacks_.size();
    }

   private:
    /**
     * @brief 启动异步回调处理线程（不需要锁，调用者必须持有锁）
     */
    void StartUnlocked() {
        if (!running_) {
            running_ = true;
            worker_thread_ = std::thread(&AsyncCallback::WorkerLoop, this);
        }
    }

    void WorkerLoop() {
        while (true) {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return !callbacks_.empty() || !running_; });

            if (!running_ && callbacks_.empty()) {
                break;
            }

            while (!callbacks_.empty()) {
                auto callback = callbacks_.front();
                callbacks_.pop();
                lock.unlock();
                callback();
                lock.lock();
            }
        }
    }

    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<std::function<void()>> callbacks_;
    std::vector<std::shared_future<void>> pending_futures_;
    std::thread worker_thread_;
    bool running_;
};

}  // namespace cAsync

#endif  // __ASYNC_CALLBACK__

