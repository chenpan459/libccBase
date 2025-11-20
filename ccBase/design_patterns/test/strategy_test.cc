#include <iostream>
#include <memory>
#include "strategy.h"

using namespace cStrategy;
using std::cout;
using std::endl;

void TestStrategy() {
    cout << "\n========== 测试1: 策略模式 ==========" << endl;

    struct PaymentData {
        double amount;
        std::string method;
    };

    class CreditCardStrategy : public Strategy<PaymentData> {
       public:
        void Execute(PaymentData& data) override {
            cout << "使用信用卡支付: $" << data.amount << endl;
            data.method = "CreditCard";
        }
    };

    class PayPalStrategy : public Strategy<PaymentData> {
       public:
        void Execute(PaymentData& data) override {
            cout << "使用PayPal支付: $" << data.amount << endl;
            data.method = "PayPal";
        }
    };

    PaymentData payment{100.0, ""};
    StrategyContext<PaymentData> context(payment);

    context.SetStrategy(std::make_shared<CreditCardStrategy>());
    context.Execute();

    context.SetStrategy(std::make_shared<PayPalStrategy>());
    context.Execute();
}

int main() {
    cout << "========================================" << endl;
    cout << "    策略模式测试程序" << endl;
    cout << "========================================" << endl;

    try {
        TestStrategy();

        cout << "\n========================================" << endl;
        cout << "    所有测试完成！" << endl;
        cout << "========================================" << endl;
    } catch (const std::exception& e) {
        cout << "测试异常: " << e.what() << endl;
        return 1;
    }

    return 0;
}

