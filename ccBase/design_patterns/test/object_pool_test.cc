#include <iostream>
#include <memory>
#include <vector>
#include "object_pool.h"

using namespace cObjectPool;
using std::cout;
using std::endl;

void TestObjectPool() {
    cout << "\n========== 测试1: 对象池基本功能 ==========" << endl;

    struct Connection {
        int id;
        bool connected;

        Connection() : id(0), connected(false) {}
        void Connect() { connected = true; }
        void Disconnect() { connected = false; }
    };

    int next_id = 1;
    auto factory = [&next_id]() {
        auto conn = std::make_unique<Connection>();
        conn->id = next_id++;
        return conn;
    };

    auto reset = [](Connection* conn) { conn->Disconnect(); };

    ObjectPool<Connection> pool(factory, reset, 5);

    // 获取对象
    std::vector<std::shared_ptr<Connection>> connections;
    for (int i = 0; i < 3; ++i) {
        auto conn = pool.Acquire();
        if (conn) {
            conn->Connect();
            connections.push_back(conn);
            cout << "获取连接 ID: " << conn->id << endl;
        }
    }

    cout << "可用对象数: " << pool.GetAvailableCount() << endl;
    cout << "总对象数: " << pool.GetTotalCount() << endl;

    // 释放对象
    connections.clear();
    cout << "释放所有连接后，可用对象数: " << pool.GetAvailableCount() << endl;
}

int main() {
    cout << "========================================" << endl;
    cout << "    对象池模式测试程序" << endl;
    cout << "========================================" << endl;

    try {
        TestObjectPool();

        cout << "\n========================================" << endl;
        cout << "    所有测试完成！" << endl;
        cout << "========================================" << endl;
    } catch (const std::exception& e) {
        cout << "测试异常: " << e.what() << endl;
        return 1;
    }

    return 0;
}

