#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include "pub_sub.h"

using namespace cPubSub;
using std::cout;
using std::endl;

// 测试消息类型
struct NewsMessage {
    std::string title;
    std::string content;
    int priority;

    NewsMessage(const std::string& t, const std::string& c, int p)
        : title(t), content(c), priority(p) {}
};

struct StockPrice {
    std::string symbol;
    double price;
    double change;

    StockPrice(const std::string& s, double p, double c) : symbol(s), price(p), change(c) {}
};

// 测试1: 基本订阅和发布
void TestBasicPubSub() {
    cout << "\n========== 测试1: 基本订阅和发布 ==========" << endl;
    PubSub<NewsMessage> pubsub;

    // 订阅者1
    auto id1 = pubsub.Subscribe("news", [](const std::string& topic, const NewsMessage& msg) {
        cout << "[订阅者1] 收到消息 - 主题: " << topic << ", 标题: " << msg.title
             << ", 内容: " << msg.content << ", 优先级: " << msg.priority << endl;
    });

    // 订阅者2
    auto id2 = pubsub.Subscribe("news", [](const std::string& topic, const NewsMessage& msg) {
        cout << "[订阅者2] 收到消息 - 主题: " << topic << ", 标题: " << msg.title << endl;
    });

    // 发布消息
    NewsMessage msg1("重要新闻", "这是一条重要新闻", 1);
    size_t count = pubsub.Publish("news", msg1);
    cout << "发布消息，收到消息的订阅者数量: " << count << endl;

    // 取消订阅
    pubsub.Unsubscribe("news", id1);
    cout << "取消订阅者1后，订阅者数量: " << pubsub.GetSubscriberCount("news") << endl;

    // 再次发布
    NewsMessage msg2("普通新闻", "这是一条普通新闻", 2);
    count = pubsub.Publish("news", msg2);
    cout << "再次发布消息，收到消息的订阅者数量: " << count << endl;
}

// 测试2: 多主题订阅
void TestMultipleTopics() {
    cout << "\n========== 测试2: 多主题订阅 ==========" << endl;
    PubSub<StockPrice> pubsub;

    // 订阅多个主题
    auto id1 = pubsub.Subscribe("stock.AAPL", [](const std::string& topic, const StockPrice& price) {
        cout << "[AAPL订阅者] " << price.symbol << " 价格: $" << price.price
             << ", 涨跌: " << price.change << "%" << endl;
    });

    auto id2 = pubsub.Subscribe("stock.GOOGL", [](const std::string& topic, const StockPrice& price) {
        cout << "[GOOGL订阅者] " << price.symbol << " 价格: $" << price.price
             << ", 涨跌: " << price.change << "%" << endl;
    });

    auto id3 = pubsub.Subscribe("stock.*", [](const std::string& topic, const StockPrice& price) {
        cout << "[所有股票订阅者] " << topic << " - " << price.symbol << " 价格: $" << price.price
             << endl;
    });

    // 发布不同主题的消息
    StockPrice aapl("AAPL", 150.25, 2.5);
    pubsub.Publish("stock.AAPL", aapl);

    StockPrice googl("GOOGL", 2800.50, -1.2);
    pubsub.Publish("stock.GOOGL", googl);

    cout << "AAPL主题订阅者数量: " << pubsub.GetSubscriberCount("stock.AAPL") << endl;
    cout << "GOOGL主题订阅者数量: " << pubsub.GetSubscriberCount("stock.GOOGL") << endl;
    cout << "总订阅者数量: " << pubsub.GetTotalSubscriberCount() << endl;
}

// 测试3: 发布到所有主题
void TestPublishToAll() {
    cout << "\n========== 测试3: 发布到所有主题 ==========" << endl;
    PubSub<std::string> pubsub;

    // 订阅不同主题
    pubsub.Subscribe("topic1", [](const std::string& topic, const std::string& msg) {
        cout << "[topic1订阅者] 收到: " << msg << endl;
    });

    pubsub.Subscribe("topic2", [](const std::string& topic, const std::string& msg) {
        cout << "[topic2订阅者] 收到: " << msg << endl;
    });

    pubsub.Subscribe("topic3", [](const std::string& topic, const std::string& msg) {
        cout << "[topic3订阅者] 收到: " << msg << endl;
    });

    // 发布到所有主题
    size_t count = pubsub.PublishToAll("广播消息：系统维护中");
    cout << "广播消息，收到消息的订阅者总数: " << count << endl;
}

// 测试4: 线程安全测试
void TestThreadSafety() {
    cout << "\n========== 测试4: 线程安全测试 ==========" << endl;
    PubSub<int> pubsub;
    std::atomic<int> received_count(0);

    // 创建多个订阅者
    std::vector<SubscriberId> ids;
    for (int i = 0; i < 5; ++i) {
        auto id = pubsub.Subscribe("numbers", [&received_count](const std::string& topic, const int& num) {
            received_count++;
            // 模拟一些处理时间
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        });
        ids.push_back(id);
    }

    // 在多个线程中发布消息
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&pubsub, i]() {
            for (int j = 0; j < 10; ++j) {
                pubsub.Publish("numbers", i * 10 + j);
            }
        });
    }

    // 等待所有发布线程完成
    for (auto& t : threads) {
        t.join();
    }

    // 等待所有消息处理完成
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    cout << "发布消息总数: 100 (10线程 × 10消息)" << endl;
    cout << "订阅者数量: 5" << endl;
    cout << "预期接收消息数: 500" << endl;
    cout << "实际接收消息数: " << received_count.load() << endl;
}

// 测试5: 取消订阅测试
void TestUnsubscribe() {
    cout << "\n========== 测试5: 取消订阅测试 ==========" << endl;
    PubSub<std::string> pubsub;

    // 创建多个订阅者
    auto id1 = pubsub.Subscribe("test", [](const std::string& topic, const std::string& msg) {
        cout << "[订阅者1] " << msg << endl;
    });

    auto id2 = pubsub.Subscribe("test", [](const std::string& topic, const std::string& msg) {
        cout << "[订阅者2] " << msg << endl;
    });

    auto id3 = pubsub.Subscribe("test", [](const std::string& topic, const std::string& msg) {
        cout << "[订阅者3] " << msg << endl;
    });

    cout << "初始订阅者数量: " << pubsub.GetSubscriberCount("test") << endl;

    // 发布消息
    pubsub.Publish("test", "消息1");

    // 取消订阅者2
    pubsub.Unsubscribe("test", id2);
    cout << "取消订阅者2后，订阅者数量: " << pubsub.GetSubscriberCount("test") << endl;

    // 再次发布
    pubsub.Publish("test", "消息2");

    // 使用UnsubscribeAll取消所有订阅
    size_t removed = pubsub.UnsubscribeAll(id1);
    cout << "取消订阅者1的所有订阅，移除数量: " << removed << endl;
    cout << "剩余订阅者数量: " << pubsub.GetSubscriberCount("test") << endl;
}

// 测试6: 主题管理测试
void TestTopicManagement() {
    cout << "\n========== 测试6: 主题管理测试 ==========" << endl;
    PubSub<int> pubsub;

    // 订阅多个主题
    pubsub.Subscribe("topic1", [](const std::string& topic, const int& num) {});
    pubsub.Subscribe("topic2", [](const std::string& topic, const int& num) {});
    pubsub.Subscribe("topic3", [](const std::string& topic, const int& num) {});

    auto topics = pubsub.GetAllTopics();
    cout << "所有主题: ";
    for (const auto& topic : topics) {
        cout << topic << " ";
    }
    cout << endl;

    cout << "topic1是否有订阅者: " << (pubsub.HasSubscribers("topic1") ? "是" : "否") << endl;
    cout << "topic4是否有订阅者: " << (pubsub.HasSubscribers("topic4") ? "是" : "否") << endl;

    pubsub.Clear();
    cout << "清空后，总订阅者数量: " << pubsub.GetTotalSubscriberCount() << endl;
}

// 测试7: 性能测试
void TestPerformance() {
    cout << "\n========== 测试7: 性能测试 ==========" << endl;
    PubSub<int> pubsub;

    // 创建大量订阅者
    const int subscriber_count = 100;
    for (int i = 0; i < subscriber_count; ++i) {
        pubsub.Subscribe("perf", [](const std::string& topic, const int& num) {
            // 空回调，仅测试性能
        });
    }

    const int message_count = 1000;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < message_count; ++i) {
        pubsub.Publish("perf", i);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    cout << "订阅者数量: " << subscriber_count << endl;
    cout << "消息数量: " << message_count << endl;
    cout << "总回调次数: " << (subscriber_count * message_count) << endl;
    cout << "总耗时: " << duration.count() << " 微秒" << endl;
    cout << "平均每次发布耗时: " << (duration.count() / message_count) << " 微秒" << endl;
}

int main() {
    cout << "========================================" << endl;
    cout << "    发布-订阅模式测试程序" << endl;
    cout << "========================================" << endl;

    try {
        TestBasicPubSub();
        TestMultipleTopics();
        TestPublishToAll();
        TestThreadSafety();
        TestUnsubscribe();
        TestTopicManagement();
        TestPerformance();

        cout << "\n========================================" << endl;
        cout << "    所有测试完成！" << endl;
        cout << "========================================" << endl;
    } catch (const std::exception& e) {
        cout << "测试异常: " << e.what() << endl;
        return 1;
    }

    return 0;
}

