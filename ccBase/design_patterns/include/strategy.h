#ifndef __STRATEGY__
#define __STRATEGY__

#include <memory>

namespace cStrategy {

/**
 * @brief 策略接口
 * @tparam Context 上下文类型
 */
template <typename Context>
class Strategy {
   public:
    virtual ~Strategy() = default;
    virtual void Execute(Context& context) = 0;
};

/**
 * @brief 策略上下文
 * @tparam Context 上下文数据类型
 */
template <typename Context>
class StrategyContext {
   public:
    StrategyContext(Context data) : data_(data) {}

    void SetStrategy(std::shared_ptr<Strategy<Context>> strategy) {
        strategy_ = strategy;
    }

    void Execute() {
        if (strategy_) {
            strategy_->Execute(data_);
        }
    }

    Context& GetData() { return data_; }
    const Context& GetData() const { return data_; }

   private:
    Context data_;
    std::shared_ptr<Strategy<Context>> strategy_;
};

}  // namespace cStrategy

#endif  // __STRATEGY__

