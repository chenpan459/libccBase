#include <iostream>
#include <memory>
#include "state_machine.h"

using namespace cStateMachine;
using std::cout;
using std::endl;

void TestStateMachine() {
    cout << "\n========== 测试1: 状态机 ==========" << endl;

    struct GameContext {
        int health = 100;
        int score = 0;
    };

    class IdleState : public State<GameContext> {
       public:
        std::string GetName() const override { return "Idle"; }
        void Enter(GameContext& ctx) override {
            cout << "进入空闲状态" << endl;
        }
    };

    class RunningState : public State<GameContext> {
       public:
        std::string GetName() const override { return "Running"; }
        void Enter(GameContext& ctx) override {
            cout << "进入运行状态" << endl;
        }
        void Update(GameContext& ctx) override {
            ctx.score += 10;
            cout << "运行中，得分: " << ctx.score << endl;
        }
    };

    class PausedState : public State<GameContext> {
       public:
        std::string GetName() const override { return "Paused"; }
        void Enter(GameContext& ctx) override {
            cout << "进入暂停状态" << endl;
        }
    };

    GameContext ctx;
    StateMachine<GameContext> sm;

    sm.AddState(std::make_shared<IdleState>());
    sm.AddState(std::make_shared<RunningState>());
    sm.AddState(std::make_shared<PausedState>());

    sm.AddTransition("Idle", "Running");
    sm.AddTransition("Running", "Paused");
    sm.AddTransition("Paused", "Running");

    sm.SetInitialState("Idle");
    sm.Start(ctx);

    sm.TransitionTo("Running", ctx);
    sm.Update(ctx);
    sm.TransitionTo("Paused", ctx);
}

int main() {
    cout << "========================================" << endl;
    cout << "    状态机模式测试程序" << endl;
    cout << "========================================" << endl;

    try {
        TestStateMachine();

        cout << "\n========================================" << endl;
        cout << "    所有测试完成！" << endl;
        cout << "========================================" << endl;
    } catch (const std::exception& e) {
        cout << "测试异常: " << e.what() << endl;
        return 1;
    }

    return 0;
}

