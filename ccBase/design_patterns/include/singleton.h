#ifndef __SINGLETON__
#define __SINGLETON__

#include <memory>
#include <mutex>

namespace cSingleton {

/**
 * @brief 线程安全的单例模式（懒汉式）
 * @tparam T 单例类型
 */
template <typename T>
class Singleton {
   public:
    static T& GetInstance() {
        std::call_once(once_flag_, []() {
            instance_ = std::unique_ptr<T>(new T());
        });
        return *instance_;
    }

    // 禁止拷贝和赋值
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

   protected:
    Singleton() = default;
    ~Singleton() = default;

   private:
    static std::unique_ptr<T> instance_;
    static std::once_flag once_flag_;
};

template <typename T>
std::unique_ptr<T> Singleton<T>::instance_ = nullptr;

template <typename T>
std::once_flag Singleton<T>::once_flag_;

/**
 * @brief 线程安全的单例模式（饿汉式）
 * @tparam T 单例类型
 */
template <typename T>
class EagerSingleton {
   public:
    static T& GetInstance() {
        static T instance;
        return instance;
    }

    // 禁止拷贝和赋值
    EagerSingleton(const EagerSingleton&) = delete;
    EagerSingleton& operator=(const EagerSingleton&) = delete;

   protected:
    EagerSingleton() = default;
    ~EagerSingleton() = default;
};

}  // namespace cSingleton

#endif  // __SINGLETON__

