#ifndef __FACTORY__
#define __FACTORY__

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace cFactory {

/**
 * @brief 简单工厂
 * @tparam Base 基类类型
 * @tparam Key 键类型
 */
template <typename Base, typename Key = std::string>
class SimpleFactory {
   public:
    using CreatorFunc = std::function<std::unique_ptr<Base>()>;

    /**
     * @brief 注册创建函数
     */
    void Register(const Key& key, CreatorFunc creator) {
        creators_[key] = creator;
    }

    /**
     * @brief 创建对象
     */
    std::unique_ptr<Base> Create(const Key& key) {
        auto it = creators_.find(key);
        if (it != creators_.end()) {
            return it->second();
        }
        return nullptr;
    }

    /**
     * @brief 检查是否已注册
     */
    bool IsRegistered(const Key& key) const {
        return creators_.find(key) != creators_.end();
    }

   private:
    std::map<Key, CreatorFunc> creators_;
};

/**
 * @brief 抽象工厂接口
 * @tparam Product 产品类型
 */
template <typename Product>
class AbstractFactory {
   public:
    virtual ~AbstractFactory() = default;
    virtual std::unique_ptr<Product> Create() = 0;
};

/**
 * @brief 具体工厂
 * @tparam Product 产品类型
 * @tparam ConcreteProduct 具体产品类型
 */
template <typename Product, typename ConcreteProduct>
class ConcreteFactory : public AbstractFactory<Product> {
   public:
    std::unique_ptr<Product> Create() override {
        return std::make_unique<ConcreteProduct>();
    }
};

}  // namespace cFactory

#endif  // __FACTORY__

