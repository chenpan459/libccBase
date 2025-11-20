#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include "observer.h"

using namespace cObserver;
using std::cout;
using std::endl;

// 测试数据类
struct WeatherData {
    float temperature;
    float humidity;
    float pressure;

    WeatherData(float t, float h, float p) : temperature(t), humidity(h), pressure(p) {}
};

struct StockData {
    std::string symbol;
    double price;
    double change;

    StockData(const std::string& s, double p, double c) : symbol(s), price(p), change(c) {}
};

// 测试1: 基本观察者模式
void TestBasicObserver() {
    cout << "\n========== 测试1: 基本观察者模式 ==========" << endl;

    Subject<WeatherData> weather_station;

    // 创建具体观察者类
    class DisplayObserver : public Observer<WeatherData> {
       public:
        DisplayObserver(const std::string& name) : name_(name) {}
        void Update(const WeatherData& data) override {
            cout << "[" << name_ << "] 温度: " << data.temperature << "°C, 湿度: "
                 << data.humidity << "%, 气压: " << data.pressure << "hPa" << endl;
        }

       private:
        std::string name_;
    };

    // 创建观察者
    auto display1 = std::make_shared<DisplayObserver>("显示屏1");
    auto display2 = std::make_shared<DisplayObserver>("显示屏2");

    // 添加观察者
    auto id1 = weather_station.Attach(display1);
    auto id2 = weather_station.Attach(display2);

    cout << "观察者数量: " << weather_station.GetObserverCount() << endl;

    // 通知观察者
    WeatherData data(25.5f, 60.0f, 1013.25f);
    size_t notified = weather_station.Notify(data);
    cout << "通知了 " << notified << " 个观察者" << endl;

    // 移除一个观察者
    weather_station.Detach(id1);
    cout << "移除观察者1后，观察者数量: " << weather_station.GetObserverCount() << endl;

    // 再次通知
    WeatherData data2(26.0f, 65.0f, 1014.0f);
    notified = weather_station.Notify(data2);
    cout << "再次通知，通知了 " << notified << " 个观察者" << endl;
}

// 测试2: 函数式观察者
void TestFunctionalObserver() {
    cout << "\n========== 测试2: 函数式观察者 ==========" << endl;

    Subject<StockData> stock_market;

    // 使用函数式观察者
    auto observer1 = MakeObserver<StockData>([](const StockData& data) {
        cout << "[交易员A] " << data.symbol << " 价格: $" << data.price
             << ", 涨跌: " << data.change << "%" << endl;
    });

    auto observer2 = MakeObserver<StockData>([](const StockData& data) {
        cout << "[交易员B] " << data.symbol << " 当前价格: $" << data.price << endl;
    });

    stock_market.Attach(observer1);
    stock_market.Attach(observer2);

    // 发布股票数据
    StockData aapl("AAPL", 150.25, 2.5);
    stock_market.Notify(aapl);

    StockData googl("GOOGL", 2800.50, -1.2);
    stock_market.Notify(googl);
}

// 测试3: 多个观察者
void TestMultipleObservers() {
    cout << "\n========== 测试3: 多个观察者 ==========" << endl;

    Subject<std::string> news_agency;

    // 创建多个观察者
    std::vector<std::shared_ptr<Observer<std::string>>> observers;
    for (int i = 1; i <= 5; ++i) {
        auto observer = MakeObserver<std::string>([i](const std::string& news) {
            cout << "[订阅者" << i << "] 收到新闻: " << news << endl;
        });
        observers.push_back(observer);
        news_agency.Attach(observer);
    }

    cout << "观察者数量: " << news_agency.GetObserverCount() << endl;

    // 发布新闻
    news_agency.Notify("重要新闻：科技股大涨");
    news_agency.Notify("突发新闻：市场波动");

    // 移除部分观察者
    news_agency.Detach(observers[0]);
    news_agency.Detach(observers[1]);
    cout << "移除2个观察者后，观察者数量: " << news_agency.GetObserverCount() << endl;

    news_agency.Notify("后续新闻：市场稳定");
}

// 测试4: 线程安全测试
void TestThreadSafety() {
    cout << "\n========== 测试4: 线程安全测试 ==========" << endl;

    Subject<int> subject;
    std::atomic<int> received_count(0);

    // 创建多个观察者
    std::vector<std::shared_ptr<Observer<int>>> observers;
    for (int i = 0; i < 10; ++i) {
        auto observer = MakeObserver<int>([&received_count](const int& value) {
            received_count++;
            // 模拟一些处理时间
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        });
        observers.push_back(observer);
        subject.Attach(observer);
    }

    // 在多个线程中通知
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&subject, i]() {
            for (int j = 0; j < 10; ++j) {
                subject.Notify(i * 10 + j);
            }
        });
    }

    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }

    // 等待所有通知处理完成
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    cout << "发布通知总数: 50 (5线程 × 10通知)" << endl;
    cout << "观察者数量: 10" << endl;
    cout << "预期接收通知数: 500" << endl;
    cout << "实际接收通知数: " << received_count.load() << endl;
}

// 测试5: 观察者生命周期管理
void TestObserverLifetime() {
    cout << "\n========== 测试5: 观察者生命周期管理 ==========" << endl;

    Subject<std::string> subject;

    {
        // 创建临时观察者
        auto temp_observer = MakeObserver<std::string>([](const std::string& msg) {
            cout << "[临时观察者] " << msg << endl;
        });
        subject.Attach(temp_observer);
        cout << "添加临时观察者后，观察者数量: " << subject.GetObserverCount() << endl;

        subject.Notify("消息1");
    }  // temp_observer 离开作用域

    // 自动清理失效的观察者
    cout << "临时观察者离开作用域后，观察者数量: " << subject.GetObserverCount() << endl;

    subject.Notify("消息2");

    // 手动清理
    subject.Clear();
    cout << "清空后，观察者数量: " << subject.GetObserverCount() << endl;
}

// 测试6: 通过指针移除观察者
void TestDetachByPointer() {
    cout << "\n========== 测试6: 通过指针移除观察者 ==========" << endl;

    Subject<int> subject;

    auto observer1 = MakeObserver<int>([](const int& value) {
        cout << "[观察者1] 收到: " << value << endl;
    });

    auto observer2 = MakeObserver<int>([](const int& value) {
        cout << "[观察者2] 收到: " << value << endl;
    });

    subject.Attach(observer1);
    subject.Attach(observer2);
    cout << "初始观察者数量: " << subject.GetObserverCount() << endl;

    subject.Notify(100);

    // 通过指针移除观察者
    size_t removed = subject.Detach(observer1);
    cout << "通过指针移除观察者1，移除数量: " << removed << endl;
    cout << "剩余观察者数量: " << subject.GetObserverCount() << endl;

    subject.Notify(200);
}

// 测试7: 实际应用场景 - 事件系统
void TestEventSystem() {
    cout << "\n========== 测试7: 实际应用场景 - 事件系统 ==========" << endl;

    // 定义事件类型
    enum class EventType { kClick, kHover, kClose };
    struct UIEvent {
        EventType type;
        std::string source;
        int x, y;

        UIEvent(EventType t, const std::string& s, int x_pos, int y_pos)
            : type(t), source(s), x(x_pos), y(y_pos) {}
    };

    Subject<UIEvent> event_manager;

    // 日志观察者
    auto logger = MakeObserver<UIEvent>([](const UIEvent& event) {
        const char* type_str = "";
        switch (event.type) {
            case EventType::kClick:
                type_str = "点击";
                break;
            case EventType::kHover:
                type_str = "悬停";
                break;
            case EventType::kClose:
                type_str = "关闭";
                break;
        }
        cout << "[日志系统] " << type_str << " 事件来自 " << event.source
             << " 位置: (" << event.x << ", " << event.y << ")" << endl;
    });

    // 统计观察者
    std::atomic<int> click_count(0);
    auto stats = MakeObserver<UIEvent>([&click_count](const UIEvent& event) {
        if (event.type == EventType::kClick) {
            click_count++;
        }
    });

    event_manager.Attach(logger);
    event_manager.Attach(stats);

    // 模拟UI事件
    event_manager.Notify(UIEvent(EventType::kClick, "按钮1", 100, 200));
    event_manager.Notify(UIEvent(EventType::kHover, "按钮2", 150, 250));
    event_manager.Notify(UIEvent(EventType::kClick, "按钮3", 200, 300));
    event_manager.Notify(UIEvent(EventType::kClose, "窗口1", 0, 0));

    cout << "点击事件统计: " << click_count.load() << " 次" << endl;
}

int main() {
    cout << "========================================" << endl;
    cout << "    观察者模式测试程序" << endl;
    cout << "========================================" << endl;

    try {
        TestBasicObserver();
        TestFunctionalObserver();
        TestMultipleObservers();
        TestThreadSafety();
        TestObserverLifetime();
        TestDetachByPointer();
        TestEventSystem();

        cout << "\n========================================" << endl;
        cout << "    所有测试完成！" << endl;
        cout << "========================================" << endl;
    } catch (const std::exception& e) {
        cout << "测试异常: " << e.what() << endl;
        return 1;
    }

    return 0;
}

