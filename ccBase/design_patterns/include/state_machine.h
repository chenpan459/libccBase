#ifndef __STATE_MACHINE__
#define __STATE_MACHINE__

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace cStateMachine {

/**
 * @brief 状态接口
 * @tparam Context 上下文类型
 */
template <typename Context>
class State {
   public:
    virtual ~State() = default;
    virtual void Enter(Context& context) {}
    virtual void Exit(Context& context) {}
    virtual void Update(Context& context) {}
    virtual std::string GetName() const = 0;
};

/**
 * @brief 状态机
 * @tparam Context 上下文类型
 */
template <typename Context>
class StateMachine {
   public:
    using TransitionFunc = std::function<bool(const Context&)>;

    /**
     * @brief 添加状态
     */
    void AddState(std::shared_ptr<State<Context>> state) {
        if (state) {
            states_[state->GetName()] = state;
        }
    }

    /**
     * @brief 添加转换
     * @param from 源状态名
     * @param to 目标状态名
     * @param condition 转换条件（可选）
     */
    void AddTransition(const std::string& from, const std::string& to,
                       TransitionFunc condition = nullptr) {
        transitions_[from][to] = condition;
    }

    /**
     * @brief 设置初始状态
     */
    void SetInitialState(const std::string& state_name) {
        if (states_.find(state_name) != states_.end()) {
            initial_state_ = state_name;
        }
    }

    /**
     * @brief 启动状态机
     */
    void Start(Context& context) {
        if (!initial_state_.empty() && states_.find(initial_state_) != states_.end()) {
            current_state_ = initial_state_;
            states_[current_state_]->Enter(context);
        }
    }

    /**
     * @brief 转换到指定状态
     */
    bool TransitionTo(const std::string& state_name, Context& context) {
        if (current_state_.empty()) {
            return false;
        }

        auto it = transitions_[current_state_].find(state_name);
        if (it != transitions_[current_state_].end()) {
            // 检查转换条件
            if (it->second && !it->second(context)) {
                return false;
            }

            // 执行状态转换
            states_[current_state_]->Exit(context);
            current_state_ = state_name;
            states_[current_state_]->Enter(context);
            return true;
        }
        return false;
    }

    /**
     * @brief 更新当前状态
     */
    void Update(Context& context) {
        if (!current_state_.empty()) {
            states_[current_state_]->Update(context);
        }
    }

    /**
     * @brief 获取当前状态名
     */
    std::string GetCurrentState() const { return current_state_; }

   private:
    std::map<std::string, std::shared_ptr<State<Context>>> states_;
    std::map<std::string, std::map<std::string, TransitionFunc>> transitions_;
    std::string current_state_;
    std::string initial_state_;
};

}  // namespace cStateMachine

#endif  // __STATE_MACHINE__

