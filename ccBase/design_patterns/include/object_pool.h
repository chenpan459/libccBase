#ifndef __OBJECT_POOL__
#define __OBJECT_POOL__

#include <functional>
#include <memory>
#include <mutex>
#include <queue>

namespace cObjectPool {

/**
 * @brief 对象池
 * @tparam T 对象类型
 */
template <typename T>
class ObjectPool {
   public:
    using ObjectPtr = std::shared_ptr<T>;
    using FactoryFunc = std::function<std::unique_ptr<T>()>;
    using ResetFunc = std::function<void(T*)>;

    /**
     * @brief 构造函数
     * @param factory 对象工厂函数
     * @param reset 对象重置函数（可选）
     * @param max_size 池的最大大小（0表示无限制）
     */
    ObjectPool(FactoryFunc factory, ResetFunc reset = nullptr, size_t max_size = 0)
        : factory_(std::move(factory)), reset_(std::move(reset)), max_size_(max_size) {}

    /**
     * @brief 从池中获取对象
     * @return 对象智能指针
     */
    ObjectPtr Acquire() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!pool_.empty()) {
            auto obj = std::move(pool_.front());
            pool_.pop();
            return ObjectPtr(obj.release(), [this](T* ptr) { Release(ptr); });
        }

        // 池为空，创建新对象
        if (max_size_ == 0 || current_size_ < max_size_) {
            current_size_++;
            auto obj = factory_();
            if (!obj) {
                current_size_--;
                return nullptr;
            }
            return ObjectPtr(obj.release(), [this](T* ptr) { Release(ptr); });
        }

        // 达到最大大小，返回nullptr
        return nullptr;
    }

    /**
     * @brief 获取池中可用对象数量
     */
    size_t GetAvailableCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return pool_.size();
    }

    /**
     * @brief 获取当前总对象数量
     */
    size_t GetTotalCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return current_size_;
    }

    /**
     * @brief 清空对象池
     */
    void Clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!pool_.empty()) {
            pool_.pop();
        }
        current_size_ = 0;
    }

   private:
    void Release(T* ptr) {
        if (!ptr) {
            return;
        }

        // 重置对象状态
        if (reset_) {
            reset_(ptr);
        }

        std::lock_guard<std::mutex> lock(mutex_);
        // 如果池未满或没有限制，将对象放回池中
        if (max_size_ == 0 || pool_.size() < max_size_) {
            pool_.push(std::unique_ptr<T>(ptr));
        } else {
            // 池已满，直接删除对象
            delete ptr;
            current_size_--;
        }
    }

    mutable std::mutex mutex_;
    std::queue<std::unique_ptr<T>> pool_;
    FactoryFunc factory_;
    ResetFunc reset_;
    size_t max_size_;
    size_t current_size_ = 0;
};

}  // namespace cObjectPool

#endif  // __OBJECT_POOL__

