#ifndef __COMMAND__
#define __COMMAND__

#include <functional>
#include <memory>
#include <stack>
#include <vector>

namespace cCommand {

/**
 * @brief 命令接口
 */
class Command {
   public:
    virtual ~Command() = default;
    virtual void Execute() = 0;
    virtual void Undo() = 0;
    virtual bool CanUndo() const { return true; }
};

/**
 * @brief 函数式命令
 */
class FunctionalCommand : public Command {
   public:
    using ExecuteFunc = std::function<void()>;
    using UndoFunc = std::function<void()>;

    FunctionalCommand(ExecuteFunc execute, UndoFunc undo = nullptr)
        : execute_func_(std::move(execute)), undo_func_(std::move(undo)) {}

    void Execute() override {
        if (execute_func_) {
            execute_func_();
        }
    }

    void Undo() override {
        if (undo_func_) {
            undo_func_();
        }
    }

    bool CanUndo() const override { return undo_func_ != nullptr; }

   private:
    ExecuteFunc execute_func_;
    UndoFunc undo_func_;
};

/**
 * @brief 命令管理器（支持撤销/重做）
 */
class CommandManager {
   public:
    /**
     * @brief 执行命令
     * @param command 命令对象
     */
    void Execute(std::shared_ptr<Command> command) {
        if (!command) {
            return;
        }
        command->Execute();
        undo_stack_.push(command);
        // 执行新命令后，清空重做栈
        while (!redo_stack_.empty()) {
            redo_stack_.pop();
        }
    }

    /**
     * @brief 撤销上一个命令
     */
    bool Undo() {
        if (undo_stack_.empty()) {
            return false;
        }
        auto command = undo_stack_.top();
        undo_stack_.pop();
        if (command->CanUndo()) {
            command->Undo();
            redo_stack_.push(command);
            return true;
        }
        return false;
    }

    /**
     * @brief 重做上一个撤销的命令
     */
    bool Redo() {
        if (redo_stack_.empty()) {
            return false;
        }
        auto command = redo_stack_.top();
        redo_stack_.pop();
        command->Execute();
        undo_stack_.push(command);
        return true;
    }

    /**
     * @brief 检查是否可以撤销
     */
    bool CanUndo() const { return !undo_stack_.empty(); }

    /**
     * @brief 检查是否可以重做
     */
    bool CanRedo() const { return !redo_stack_.empty(); }

    /**
     * @brief 清空所有命令历史
     */
    void Clear() {
        while (!undo_stack_.empty()) {
            undo_stack_.pop();
        }
        while (!redo_stack_.empty()) {
            redo_stack_.pop();
        }
    }

   private:
    std::stack<std::shared_ptr<Command>> undo_stack_;
    std::stack<std::shared_ptr<Command>> redo_stack_;
};

/**
 * @brief 宏命令（组合多个命令）
 */
class MacroCommand : public Command {
   public:
    void AddCommand(std::shared_ptr<Command> command) {
        if (command) {
            commands_.push_back(command);
        }
    }

    void Execute() override {
        for (auto& cmd : commands_) {
            cmd->Execute();
        }
    }

    void Undo() override {
        // 逆序撤销
        for (auto it = commands_.rbegin(); it != commands_.rend(); ++it) {
            if ((*it)->CanUndo()) {
                (*it)->Undo();
            }
        }
    }

   private:
    std::vector<std::shared_ptr<Command>> commands_;
};

}  // namespace cCommand

#endif  // __COMMAND__

