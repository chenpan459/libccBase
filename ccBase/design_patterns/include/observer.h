#ifndef __OBSERVER__
#define __OBSERVER__

#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace cObserver {

/**
 * @brief 观察者接口
 * @tparam T 通知数据的类型
 */
template <typename T>
class Observer {
   public:
    virtual ~Observer() = default;
    /**
     * @brief 接收通知的接口
     * @param data 通知数据
     */
    virtual void Update(const T& data) = 0;
};

/**
 * @brief 观察者ID类型
 */
using ObserverId = uint64_t;

/**
 * @brief 观察者包装器，用于管理观察者
 * @tparam T 通知数据的类型
 */
template <typename T>
struct ObserverWrapper {
    ObserverId id;
    std::weak_ptr<Observer<T>> observer;

    ObserverWrapper(ObserverId id, std::weak_ptr<Observer<T>> obs) : id(id), observer(obs) {}
};

/**
 * @brief 主题（被观察者）类
 * @tparam T 通知数据的类型
 */
template <typename T>
class Subject {
   public:
    Subject() : next_observer_id_(1) {}
    virtual ~Subject() = default;

    /**
     * @brief 添加观察者
     * @param observer 观察者智能指针
     * @return 观察者ID，用于移除观察者
     */
    ObserverId Attach(std::shared_ptr<Observer<T>> observer) {
        if (!observer) {
            return 0;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        ObserverId id = next_observer_id_++;
        observers_.emplace_back(id, std::weak_ptr<Observer<T>>(observer));
        return id;
    }

    /**
     * @brief 移除观察者
     * @param observer_id 观察者ID
     * @return 是否成功移除
     */
    bool Detach(ObserverId observer_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = std::remove_if(observers_.begin(), observers_.end(),
                                 [observer_id](const ObserverWrapper<T>& wrapper) {
                                     return wrapper.id == observer_id;
                                 });
        if (it != observers_.end()) {
            observers_.erase(it, observers_.end());
            return true;
        }
        return false;
    }

    /**
     * @brief 移除观察者（通过观察者指针）
     * @param observer 观察者指针
     * @return 移除的观察者数量
     */
    size_t Detach(std::shared_ptr<Observer<T>> observer) {
        if (!observer) {
            return 0;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        size_t count = 0;
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                          [&observer, &count](ObserverWrapper<T>& wrapper) {
                              auto locked = wrapper.observer.lock();
                              if (locked == observer) {
                                  count++;
                                  return true;
                              }
                              return false;
                          }),
            observers_.end());
        return count;
    }

    /**
     * @brief 通知所有观察者
     * @param data 通知数据
     * @return 实际通知的观察者数量
     */
    size_t Notify(const T& data) {
        // 先复制观察者列表，避免在回调时持有锁
        std::vector<ObserverWrapper<T>> observers_copy;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            // 清理已失效的观察者
            observers_.erase(
                std::remove_if(observers_.begin(), observers_.end(),
                               [](const ObserverWrapper<T>& wrapper) {
                                   return wrapper.observer.expired();
                               }),
                observers_.end());
            observers_copy = observers_;
        }

        // 在锁外通知观察者
        size_t notified_count = 0;
        for (auto& wrapper : observers_copy) {
            auto observer = wrapper.observer.lock();
            if (observer) {
                try {
                    observer->Update(data);
                    notified_count++;
                } catch (...) {
                    // 忽略观察者回调中的异常，避免影响其他观察者
                }
            }
        }
        return notified_count;
    }

    /**
     * @brief 获取观察者数量
     * @return 观察者数量（不包括已失效的）
     */
    size_t GetObserverCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t count = 0;
        for (const auto& wrapper : observers_) {
            if (!wrapper.observer.expired()) {
                count++;
            }
        }
        return count;
    }

    /**
     * @brief 清空所有观察者
     */
    void Clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        observers_.clear();
    }

   protected:
    mutable std::mutex mutex_;
    std::vector<ObserverWrapper<T>> observers_;
    ObserverId next_observer_id_;
};

/**
 * @brief 函数式观察者适配器
 * 允许使用函数或lambda作为观察者
 * @tparam T 通知数据的类型
 */
template <typename T>
class FunctionalObserver : public Observer<T> {
   public:
    using UpdateFunc = std::function<void(const T&)>;

    explicit FunctionalObserver(UpdateFunc func) : update_func_(std::move(func)) {}

    void Update(const T& data) override {
        if (update_func_) {
            update_func_(data);
        }
    }

   private:
    UpdateFunc update_func_;
};

/**
 * @brief 便捷函数：创建函数式观察者
 * @tparam T 通知数据的类型
 * @param func 更新函数
 * @return 观察者智能指针
 */
template <typename T>
std::shared_ptr<Observer<T>> MakeObserver(typename FunctionalObserver<T>::UpdateFunc func) {
    return std::make_shared<FunctionalObserver<T>>(std::move(func));
}

}  // namespace cObserver

#endif  // __OBSERVER__

