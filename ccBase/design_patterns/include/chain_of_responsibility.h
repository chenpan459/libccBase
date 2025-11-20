#ifndef __CHAIN_OF_RESPONSIBILITY__
#define __CHAIN_OF_RESPONSIBILITY__

#include <functional>
#include <memory>
#include <string>

namespace cChain {

/**
 * @brief 处理结果枚举
 */
enum class HandleResult {
    kHandled,      // 已处理，停止传递
    kNotHandled,   // 未处理，继续传递
    kError         // 处理出错，停止传递
};

/**
 * @brief 责任链处理者接口
 * @tparam Request 请求类型
 * @tparam Response 响应类型（可选）
 */
template <typename Request, typename Response = void>
class Handler {
   public:
    virtual ~Handler() = default;

    /**
     * @brief 处理请求
     * @param request 请求对象
     * @param response 响应对象（如果 Response 不是 void）
     * @return 处理结果
     */
    virtual HandleResult Handle(const Request& request, Response* response = nullptr) = 0;

    /**
     * @brief 设置下一个处理者
     * @param next 下一个处理者
     */
    void SetNext(std::shared_ptr<Handler<Request, Response>> next) {
        next_handler_ = next;
    }

    /**
     * @brief 获取下一个处理者
     * @return 下一个处理者
     */
    std::shared_ptr<Handler<Request, Response>> GetNext() const {
        return next_handler_;
    }

    /**
     * @brief 处理请求并传递到下一个处理者
     * @param request 请求对象
     * @param response 响应对象（如果 Response 不是 void）
     * @return 处理结果
     */
    HandleResult HandleAndPass(const Request& request, Response* response = nullptr) {
        HandleResult result = Handle(request, response);
        if (result == HandleResult::kNotHandled && next_handler_) {
            return next_handler_->HandleAndPass(request, response);
        }
        return result;
    }

   protected:
    std::shared_ptr<Handler<Request, Response>> next_handler_;
};

/**
 * @brief 函数式处理者适配器
 * @tparam Request 请求类型
 * @tparam Response 响应类型（可选）
 */
template <typename Request, typename Response = void>
class FunctionalHandler : public Handler<Request, Response> {
   public:
    using HandleFunc = std::function<HandleResult(const Request&, Response*)>;

    explicit FunctionalHandler(HandleFunc func) : handle_func_(std::move(func)) {}

    HandleResult Handle(const Request& request, Response* response = nullptr) override {
        if (handle_func_) {
            return handle_func_(request, response);
        }
        return HandleResult::kNotHandled;
    }

   private:
    HandleFunc handle_func_;
};

/**
 * @brief 便捷函数：创建函数式处理者
 * @tparam Request 请求类型
 * @tparam Response 响应类型（可选）
 * @param func 处理函数
 * @return 处理者智能指针
 */
template <typename Request, typename Response = void>
std::shared_ptr<Handler<Request, Response>> MakeHandler(
    typename FunctionalHandler<Request, Response>::HandleFunc func) {
    return std::make_shared<FunctionalHandler<Request, Response>>(std::move(func));
}

/**
 * @brief 责任链构建器
 * @tparam Request 请求类型
 * @tparam Response 响应类型（可选）
 */
template <typename Request, typename Response = void>
class ChainBuilder {
   public:
    /**
     * @brief 添加处理者到链中
     * @param handler 处理者
     * @return 构建器自身，支持链式调用
     */
    ChainBuilder& Add(std::shared_ptr<Handler<Request, Response>> handler) {
        if (!handler) {
            return *this;
        }

        if (!first_handler_) {
            first_handler_ = handler;
            current_handler_ = handler;
        } else {
            current_handler_->SetNext(handler);
            current_handler_ = handler;
        }
        return *this;
    }

    /**
     * @brief 构建责任链
     * @return 链的第一个处理者
     */
    std::shared_ptr<Handler<Request, Response>> Build() {
        return first_handler_;
    }

    /**
     * @brief 清空链
     */
    void Clear() {
        first_handler_.reset();
        current_handler_.reset();
    }

   private:
    std::shared_ptr<Handler<Request, Response>> first_handler_;
    std::shared_ptr<Handler<Request, Response>> current_handler_;
};

/**
 * @brief 责任链管理器
 * @tparam Request 请求类型
 * @tparam Response 响应类型（可选）
 */
template <typename Request, typename Response = void>
class ChainManager {
   public:
    /**
     * @brief 设置责任链
     * @param chain 责任链的第一个处理者
     */
    void SetChain(std::shared_ptr<Handler<Request, Response>> chain) {
        chain_ = chain;
    }

    /**
     * @brief 处理请求
     * @param request 请求对象
     * @param response 响应对象（如果 Response 不是 void）
     * @return 处理结果
     */
    HandleResult Process(const Request& request, Response* response = nullptr) {
        if (!chain_) {
            return HandleResult::kNotHandled;
        }
        return chain_->HandleAndPass(request, response);
    }

    /**
     * @brief 检查是否有处理链
     * @return 是否有处理链
     */
    bool HasChain() const {
        return chain_ != nullptr;
    }

   private:
    std::shared_ptr<Handler<Request, Response>> chain_;
};

}  // namespace cChain

#endif  // __CHAIN_OF_RESPONSIBILITY__

