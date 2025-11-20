#ifndef __PUB_SUB__
#define __PUB_SUB__

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace cPubSub {

/**
 * @brief 订阅者ID类型
 */
using SubscriberId = uint64_t;

/**
 * @brief 消息回调函数类型
 * @tparam T 消息数据类型
 */
template <typename T>
using MessageCallback = std::function<void(const std::string& topic, const T& message)>;

/**
 * @brief 订阅者信息
 * @tparam T 消息数据类型
 */
template <typename T>
struct Subscriber {
    SubscriberId id;
    MessageCallback<T> callback;
    std::string topic;

    Subscriber(SubscriberId id, const std::string& topic, MessageCallback<T> callback)
        : id(id), topic(topic), callback(std::move(callback)) {}
};

/**
 * @brief 发布-订阅模式实现
 * @tparam T 消息数据类型
 */
template <typename T>
class PubSub {
   public:
    PubSub() : next_subscriber_id_(1) {}

    ~PubSub() {
        std::lock_guard<std::mutex> lock(mutex_);
        subscribers_.clear();
    }

    /**
     * @brief 订阅主题
     * @param topic 主题名称
     * @param callback 消息回调函数
     * @return 订阅者ID，用于取消订阅
     */
    SubscriberId Subscribe(const std::string& topic, MessageCallback<T> callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        SubscriberId id = next_subscriber_id_++;
        auto subscriber = std::make_shared<Subscriber<T>>(id, topic, std::move(callback));
        subscribers_[topic].push_back(subscriber);
        return id;
    }

    /**
     * @brief 取消订阅
     * @param topic 主题名称
     * @param subscriber_id 订阅者ID
     * @return 是否成功取消订阅
     */
    bool Unsubscribe(const std::string& topic, SubscriberId subscriber_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subscribers_.find(topic);
        if (it == subscribers_.end()) {
            return false;
        }

        auto& subscribers = it->second;
        for (auto sub_it = subscribers.begin(); sub_it != subscribers.end(); ++sub_it) {
            if ((*sub_it)->id == subscriber_id) {
                subscribers.erase(sub_it);
                if (subscribers.empty()) {
                    subscribers_.erase(it);
                }
                return true;
            }
        }
        return false;
    }

    /**
     * @brief 取消所有订阅（通过订阅者ID）
     * @param subscriber_id 订阅者ID
     * @return 取消的订阅数量
     */
    size_t UnsubscribeAll(SubscriberId subscriber_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t count = 0;
        std::vector<std::string> empty_topics;

        for (auto& [topic, subscribers] : subscribers_) {
            for (auto it = subscribers.begin(); it != subscribers.end();) {
                if ((*it)->id == subscriber_id) {
                    it = subscribers.erase(it);
                    count++;
                } else {
                    ++it;
                }
            }
            if (subscribers.empty()) {
                empty_topics.push_back(topic);
            }
        }

        // 清理空主题
        for (const auto& topic : empty_topics) {
            subscribers_.erase(topic);
        }

        return count;
    }

    /**
     * @brief 发布消息到指定主题
     * @param topic 主题名称
     * @param message 消息内容
     * @return 接收到消息的订阅者数量
     */
    size_t Publish(const std::string& topic, const T& message) {
        std::vector<std::shared_ptr<Subscriber<T>>> subscribers_copy;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = subscribers_.find(topic);
            if (it == subscribers_.end()) {
                return 0;
            }
            // 复制订阅者列表，避免在回调时持有锁
            subscribers_copy = it->second;
        }

        // 在锁外执行回调
        for (const auto& subscriber : subscribers_copy) {
            try {
                subscriber->callback(topic, message);
            } catch (...) {
                // 忽略回调中的异常，避免影响其他订阅者
            }
        }

        return subscribers_copy.size();
    }

    /**
     * @brief 发布消息到所有主题
     * @param message 消息内容
     * @return 接收到消息的订阅者总数
     */
    size_t PublishToAll(const T& message) {
        std::unordered_map<std::string, std::vector<std::shared_ptr<Subscriber<T>>>> subscribers_copy;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            subscribers_copy = subscribers_;
        }

        size_t total_count = 0;
        // 在锁外执行回调
        for (const auto& [topic, subscribers] : subscribers_copy) {
            for (const auto& subscriber : subscribers) {
                try {
                    subscriber->callback(topic, message);
                    total_count++;
                } catch (...) {
                    // 忽略回调中的异常
                }
            }
        }

        return total_count;
    }

    /**
     * @brief 获取指定主题的订阅者数量
     * @param topic 主题名称
     * @return 订阅者数量
     */
    size_t GetSubscriberCount(const std::string& topic) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subscribers_.find(topic);
        if (it == subscribers_.end()) {
            return 0;
        }
        return it->second.size();
    }

    /**
     * @brief 获取所有主题的订阅者总数
     * @return 订阅者总数
     */
    size_t GetTotalSubscriberCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        size_t total = 0;
        for (const auto& [topic, subscribers] : subscribers_) {
            total += subscribers.size();
        }
        return total;
    }

    /**
     * @brief 获取所有主题列表
     * @return 主题名称列表
     */
    std::vector<std::string> GetAllTopics() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::string> topics;
        topics.reserve(subscribers_.size());
        for (const auto& [topic, subscribers] : subscribers_) {
            topics.push_back(topic);
        }
        return topics;
    }

    /**
     * @brief 检查主题是否存在订阅者
     * @param topic 主题名称
     * @return 是否存在订阅者
     */
    bool HasSubscribers(const std::string& topic) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = subscribers_.find(topic);
        return it != subscribers_.end() && !it->second.empty();
    }

    /**
     * @brief 清空所有订阅
     */
    void Clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        subscribers_.clear();
    }

   private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Subscriber<T>>>> subscribers_;
    SubscriberId next_subscriber_id_;
};

}  // namespace cPubSub

#endif  // __PUB_SUB__

