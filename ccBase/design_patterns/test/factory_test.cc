#include <iostream>
#include <memory>
#include "factory.h"

using namespace cFactory;
using std::cout;
using std::endl;

void TestSimpleFactory() {
    cout << "\n========== 测试1: 简单工厂 ==========" << endl;

    class Animal {
       public:
        virtual ~Animal() = default;
        virtual void Speak() = 0;
    };

    class Dog : public Animal {
       public:
        void Speak() override { cout << "汪汪!" << endl; }
    };

    class Cat : public Animal {
       public:
        void Speak() override { cout << "喵喵!" << endl; }
    };

    SimpleFactory<Animal> factory;
    factory.Register("dog", []() { return std::make_unique<Dog>(); });
    factory.Register("cat", []() { return std::make_unique<Cat>(); });

    auto dog = factory.Create("dog");
    if (dog) dog->Speak();

    auto cat = factory.Create("cat");
    if (cat) cat->Speak();
}

void TestAbstractFactory() {
    cout << "\n========== 测试2: 抽象工厂 ==========" << endl;

    class Button {
       public:
        virtual ~Button() = default;
        virtual void Render() = 0;
    };

    class WindowsButton : public Button {
       public:
        void Render() override { cout << "渲染 Windows 按钮" << endl; }
    };

    class LinuxButton : public Button {
       public:
        void Render() override { cout << "渲染 Linux 按钮" << endl; }
    };

    ConcreteFactory<Button, WindowsButton> windows_factory;
    ConcreteFactory<Button, LinuxButton> linux_factory;

    auto win_btn = windows_factory.Create();
    win_btn->Render();

    auto linux_btn = linux_factory.Create();
    linux_btn->Render();
}

int main() {
    cout << "========================================" << endl;
    cout << "    工厂模式测试程序" << endl;
    cout << "========================================" << endl;

    try {
        TestSimpleFactory();
        TestAbstractFactory();

        cout << "\n========================================" << endl;
        cout << "    所有测试完成！" << endl;
        cout << "========================================" << endl;
    } catch (const std::exception& e) {
        cout << "测试异常: " << e.what() << endl;
        return 1;
    }

    return 0;
}

