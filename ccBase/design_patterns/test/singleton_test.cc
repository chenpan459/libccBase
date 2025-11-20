#include <iostream>
#include <thread>
#include "singleton.h"

using namespace cSingleton;
using std::cout;
using std::endl;

class MySingleton : public Singleton<MySingleton> {
    friend class Singleton<MySingleton>;

   public:
    void DoSomething() { cout << "执行操作，值: " << value_ << endl; }
    void SetValue(int v) { value_ = v; }
    int GetValue() const { return value_; }

   private:
    MySingleton() : value_(0) {}
    int value_;
};

class MyEagerSingleton : public EagerSingleton<MyEagerSingleton> {
    friend class EagerSingleton<MyEagerSingleton>;

   public:
    void DoSomething() { cout << "饿汉式单例，值: " << value_ << endl; }
    void SetValue(int v) { value_ = v; }

   private:
    MyEagerSingleton() : value_(100) {}
    int value_;
};

void TestSingleton() {
    cout << "\n========== 测试1: 懒汉式单例 ==========" << endl;

    auto& instance1 = MySingleton::GetInstance();
    auto& instance2 = MySingleton::GetInstance();

    instance1.SetValue(42);
    cout << "instance1 地址: " << &instance1 << endl;
    cout << "instance2 地址: " << &instance2 << endl;
    cout << "是否为同一实例: " << (&instance1 == &instance2 ? "是" : "否") << endl;
    instance2.DoSomething();
}

void TestEagerSingleton() {
    cout << "\n========== 测试2: 饿汉式单例 ==========" << endl;

    auto& instance1 = MyEagerSingleton::GetInstance();
    auto& instance2 = MyEagerSingleton::GetInstance();

    cout << "instance1 地址: " << &instance1 << endl;
    cout << "instance2 地址: " << &instance2 << endl;
    cout << "是否为同一实例: " << (&instance1 == &instance2 ? "是" : "否") << endl;
    instance1.DoSomething();
}

int main() {
    cout << "========================================" << endl;
    cout << "    单例模式测试程序" << endl;
    cout << "========================================" << endl;

    try {
        TestSingleton();
        TestEagerSingleton();

        cout << "\n========================================" << endl;
        cout << "    所有测试完成！" << endl;
        cout << "========================================" << endl;
    } catch (const std::exception& e) {
        cout << "测试异常: " << e.what() << endl;
        return 1;
    }

    return 0;
}

