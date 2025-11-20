#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include "async_callback.h"

using namespace cAsync;
using std::cout;
using std::endl;

void TestBasicAsyncCallback() {
    cout << "\n========== 测试12: 基本异步回调 ==========" << endl;

    AsyncCallback async;

    std::atomic<int> counter(0);

    // 提交多个异步任务
    for (int i = 0; i < 10; ++i) {
        async.Post([i, &counter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            counter++;
            cout << "任务 " << i << " 完成" << endl;
        });
    }
    
    // 等待所有任务完成
    async.WaitAll();
    cout << "所有任务完成，计数器: " << counter.load() << endl;
}

void TestAsyncCallbackWithReturn() {
    cout << "\n========== 测试2: 带返回值的异步回调 ==========" << endl;

    AsyncCallback async;

    // 提交带返回值的任务
    auto future1 = async.Post<int>([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        return 42;
    });

    auto future2 = async.Post<std::string>([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        return std::string("Hello, Async!");
    });

    // 获取结果
    cout << "结果1: " << future1.get() << endl;
    cout << "结果2: " << future2.get() << endl;
}

void TestAsyncCallbackException() {
    cout << "\n========== 测试3: 异常处理 ==========" << endl;

    AsyncCallback async;

    auto future = async.Post([]() {
        throw std::runtime_error("测试异常");
    });

    try {
        future.get();
    } catch (const std::exception& e) {
        cout << "捕获异常: " << e.what() << endl;
    }
}

int main() {
    cout << "========================================" << endl;
    cout << "    异步回调模式测试程序" << endl;
    cout << "========================================" << endl;

    try {
        TestBasicAsyncCallback();
        TestAsyncCallbackWithReturn();
        TestAsyncCallbackException();

        cout << "\n========================================" << endl;
        cout << "    所有测试完成！" << endl;
        cout << "========================================" << endl;
    } catch (const std::exception& e) {
        cout << "测试异常: " << e.what() << endl;
        return 1;
    }

    return 0;
}

