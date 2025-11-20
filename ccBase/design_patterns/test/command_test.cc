#include <iostream>
#include <memory>
#include "command.h"

using namespace cCommand;
using std::cout;
using std::endl;

void TestBasicCommand() {
    cout << "\n========== 测试1: 基本命令模式 ==========" << endl;

    int value = 0;

    auto cmd1 = std::make_shared<FunctionalCommand>(
        [&value]() { value += 10; cout << "执行: value += 10, 当前值: " << value << endl; },
        [&value]() { value -= 10; cout << "撤销: value -= 10, 当前值: " << value << endl; });

    auto cmd2 = std::make_shared<FunctionalCommand>(
        [&value]() { value *= 2; cout << "执行: value *= 2, 当前值: " << value << endl; },
        [&value]() { value /= 2; cout << "撤销: value /= 2, 当前值: " << value << endl; });

    CommandManager manager;
    manager.Execute(cmd1);
    manager.Execute(cmd2);

    cout << "最终值: " << value << endl;
    manager.Undo();
    cout << "撤销后值: " << value << endl;
    manager.Redo();
    cout << "重做后值: " << value << endl;
}

void TestMacroCommand() {
    cout << "\n========== 测试2: 宏命令 ==========" << endl;

    std::string text = "";

    auto macro = std::make_shared<MacroCommand>();
    macro->AddCommand(std::make_shared<FunctionalCommand>(
        [&text]() { text += "Hello "; },
        [&text]() { text = text.substr(0, text.length() - 6); }));
    macro->AddCommand(std::make_shared<FunctionalCommand>(
        [&text]() { text += "World"; },
        [&text]() { text = text.substr(0, text.length() - 5); }));

    macro->Execute();
    cout << "执行宏命令后: " << text << endl;
    macro->Undo();
    cout << "撤销宏命令后: " << text << endl;
}

int main() {
    cout << "========================================" << endl;
    cout << "    命令模式测试程序" << endl;
    cout << "========================================" << endl;

    try {
        TestBasicCommand();
        TestMacroCommand();

        cout << "\n========================================" << endl;
        cout << "    所有测试完成！" << endl;
        cout << "========================================" << endl;
    } catch (const std::exception& e) {
        cout << "测试异常: " << e.what() << endl;
        return 1;
    }

    return 0;
}

