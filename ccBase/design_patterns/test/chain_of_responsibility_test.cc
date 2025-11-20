#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "chain_of_responsibility.h"

using namespace cChain;
using std::cout;
using std::endl;

// 测试1: 基本责任链模式
void TestBasicChain() {
    cout << "\n========== 测试1: 基本责任链模式 ==========" << endl;

    // 定义请求类型
    struct PurchaseRequest {
        int amount;
        std::string purpose;

        PurchaseRequest(int a, const std::string& p) : amount(a), purpose(p) {}
    };

    // 创建具体处理者
    class ManagerHandler : public Handler<PurchaseRequest> {
       public:
        ManagerHandler(const std::string& name, int max_amount)
            : name_(name), max_amount_(max_amount) {}

        HandleResult Handle(const PurchaseRequest& request, void*) override {
            if (request.amount <= max_amount_) {
                cout << "[" << name_ << "] 批准了 " << request.amount
                     << " 元的采购请求: " << request.purpose << endl;
                return HandleResult::kHandled;
            }
            cout << "[" << name_ << "] 无法处理 " << request.amount
                 << " 元的请求，转交给上级" << endl;
            return HandleResult::kNotHandled;
        }

       private:
        std::string name_;
        int max_amount_;
    };

    // 构建责任链
    auto manager = std::make_shared<ManagerHandler>("部门经理", 1000);
    auto director = std::make_shared<ManagerHandler>("总监", 5000);
    auto ceo = std::make_shared<ManagerHandler>("CEO", 10000);

    manager->SetNext(director);
    director->SetNext(ceo);

    // 处理请求
    PurchaseRequest req1(500, "购买办公用品");
    manager->HandleAndPass(req1);

    PurchaseRequest req2(3000, "购买设备");
    manager->HandleAndPass(req2);

    PurchaseRequest req3(8000, "项目投资");
    manager->HandleAndPass(req3);

    PurchaseRequest req4(15000, "大型项目");
    HandleResult result = manager->HandleAndPass(req4);
    if (result == HandleResult::kNotHandled) {
        cout << "所有处理者都无法处理该请求" << endl;
    }
}

// 测试2: 使用函数式处理者
void TestFunctionalHandler() {
    cout << "\n========== 测试2: 使用函数式处理者 ==========" << endl;

    struct LogRequest {
        std::string level;
        std::string message;

        LogRequest(const std::string& l, const std::string& m) : level(l), message(m) {}
    };

    // 创建函数式处理者
    auto console_handler = MakeHandler<LogRequest>([](const LogRequest& req, void*) {
        if (req.level == "INFO" || req.level == "DEBUG") {
            cout << "[控制台] " << req.message << endl;
            return HandleResult::kHandled;
        }
        return HandleResult::kNotHandled;
    });

    auto file_handler = MakeHandler<LogRequest>([](const LogRequest& req, void*) {
        if (req.level == "WARNING" || req.level == "ERROR") {
            cout << "[文件日志] " << req.level << ": " << req.message << endl;
            return HandleResult::kHandled;
        }
        return HandleResult::kNotHandled;
    });

    auto email_handler = MakeHandler<LogRequest>([](const LogRequest& req, void*) {
        if (req.level == "ERROR" || req.level == "CRITICAL") {
            cout << "[邮件通知] 严重错误: " << req.message << endl;
            return HandleResult::kHandled;
        }
        return HandleResult::kNotHandled;
    });

    // 构建链
    console_handler->SetNext(file_handler);
    file_handler->SetNext(email_handler);

    // 处理日志请求
    LogRequest log1("DEBUG", "调试信息");
    console_handler->HandleAndPass(log1);

    LogRequest log2("WARNING", "警告信息");
    console_handler->HandleAndPass(log2);

    LogRequest log3("ERROR", "错误信息");
    console_handler->HandleAndPass(log3);
}

// 测试3: 使用链构建器
void TestChainBuilder() {
    cout << "\n========== 测试3: 使用链构建器 ==========" << endl;

    struct ValidationRequest {
        std::string username;
        std::string password;
        std::string email;

        ValidationRequest(const std::string& u, const std::string& p, const std::string& e)
            : username(u), password(p), email(e) {}
    };

    // 使用链构建器
    ChainBuilder<ValidationRequest> builder;

    builder.Add(MakeHandler<ValidationRequest>([](const ValidationRequest& req, void*) {
        if (req.username.empty()) {
            cout << "[验证器1] 用户名不能为空" << endl;
            return HandleResult::kError;
        }
        cout << "[验证器1] 用户名验证通过" << endl;
        return HandleResult::kNotHandled;
    }));

    builder.Add(MakeHandler<ValidationRequest>([](const ValidationRequest& req, void*) {
        if (req.password.length() < 6) {
            cout << "[验证器2] 密码长度不足" << endl;
            return HandleResult::kError;
        }
        cout << "[验证器2] 密码验证通过" << endl;
        return HandleResult::kNotHandled;
    }));

    builder.Add(MakeHandler<ValidationRequest>([](const ValidationRequest& req, void*) {
        if (req.email.find('@') == std::string::npos) {
            cout << "[验证器3] 邮箱格式不正确" << endl;
            return HandleResult::kError;
        }
        cout << "[验证器3] 邮箱验证通过" << endl;
        return HandleResult::kHandled;
    }));

    auto chain = builder.Build();

    // 测试验证
    ValidationRequest valid_req("user123", "password123", "user@example.com");
    HandleResult result = chain->HandleAndPass(valid_req);
    if (result == HandleResult::kHandled) {
        cout << "所有验证通过！" << endl;
    }

    ValidationRequest invalid_req("", "pass", "invalid-email");
    result = chain->HandleAndPass(invalid_req);
    if (result == HandleResult::kError) {
        cout << "验证失败！" << endl;
    }
}

// 测试4: 带响应的责任链
void TestChainWithResponse() {
    cout << "\n========== 测试4: 带响应的责任链 ==========" << endl;

    struct HttpRequest {
        std::string method;
        std::string path;
        std::string body;

        HttpRequest() = default;
        HttpRequest(const std::string& m, const std::string& p, const std::string& b)
            : method(m), path(p), body(b) {}
    };

    struct HttpResponse {
        int status_code;
        std::string body;
        std::string content_type;

        HttpResponse() : status_code(200), content_type("text/plain") {}
    };

    // 创建处理者
    auto static_handler = MakeHandler<HttpRequest, HttpResponse>(
        [](const HttpRequest& req, HttpResponse* resp) {
            if (req.path.find("/static/") == 0) {
                resp->status_code = 200;
                resp->body = "静态文件内容";
                resp->content_type = "text/html";
                cout << "[静态文件处理器] 处理静态文件请求" << endl;
                return HandleResult::kHandled;
            }
            return HandleResult::kNotHandled;
        });

    auto api_handler = MakeHandler<HttpRequest, HttpResponse>(
        [](const HttpRequest& req, HttpResponse* resp) {
            if (req.path.find("/api/") == 0) {
                resp->status_code = 200;
                resp->body = R"({"status": "ok"})";
                resp->content_type = "application/json";
                cout << "[API处理器] 处理API请求" << endl;
                return HandleResult::kHandled;
            }
            return HandleResult::kNotHandled;
        });

    auto not_found_handler = MakeHandler<HttpRequest, HttpResponse>(
        [](const HttpRequest& req, HttpResponse* resp) {
            resp->status_code = 404;
            resp->body = "Not Found";
            resp->content_type = "text/plain";
            cout << "[404处理器] 处理未找到的请求" << endl;
            return HandleResult::kHandled;
        });

    // 构建链
    static_handler->SetNext(api_handler);
    api_handler->SetNext(not_found_handler);

    // 处理请求
    HttpRequest req1("GET", "/static/index.html", "");
    HttpResponse resp1;
    static_handler->HandleAndPass(req1, &resp1);
    cout << "响应: " << resp1.status_code << " " << resp1.body << endl;

    HttpRequest req2("GET", "/api/users", "");
    HttpResponse resp2;
    api_handler->HandleAndPass(req2, &resp2);
    cout << "响应: " << resp2.status_code << " " << resp2.body << endl;

    HttpRequest req3("GET", "/unknown", "");
    HttpResponse resp3;
    static_handler->HandleAndPass(req3, &resp3);
    cout << "响应: " << resp3.status_code << " " << resp3.body << endl;
}

// 测试5: 使用链管理器
void TestChainManager() {
    cout << "\n========== 测试5: 使用链管理器 ==========" << endl;

    struct AuthenticationRequest {
        std::string token;
        std::string resource;

        AuthenticationRequest(const std::string& t, const std::string& r)
            : token(t), resource(r) {}
    };

    // 构建认证链
    ChainBuilder<AuthenticationRequest> builder;

    builder.Add(MakeHandler<AuthenticationRequest>([](const AuthenticationRequest& req, void*) {
        if (req.token.empty()) {
            cout << "[认证器1] Token为空，拒绝访问" << endl;
            return HandleResult::kError;
        }
        cout << "[认证器1] Token验证通过" << endl;
        return HandleResult::kNotHandled;
    }));

    builder.Add(MakeHandler<AuthenticationRequest>([](const AuthenticationRequest& req, void*) {
        if (req.token == "admin_token") {
            cout << "[认证器2] 管理员权限验证通过" << endl;
            return HandleResult::kHandled;
        }
        cout << "[认证器2] 普通用户权限验证通过" << endl;
        return HandleResult::kHandled;
    }));

    // 使用链管理器
    ChainManager<AuthenticationRequest> manager;
    manager.SetChain(builder.Build());

    // 处理请求
    AuthenticationRequest req1("admin_token", "/admin/users");
    HandleResult result = manager.Process(req1);
    cout << "处理结果: " << (result == HandleResult::kHandled ? "成功" : "失败") << endl;

    AuthenticationRequest req2("user_token", "/api/data");
    result = manager.Process(req2);
    cout << "处理结果: " << (result == HandleResult::kHandled ? "成功" : "失败") << endl;

    AuthenticationRequest req3("", "/api/data");
    result = manager.Process(req3);
    cout << "处理结果: " << (result == HandleResult::kError ? "错误" : "成功") << endl;
}

// 测试6: 实际应用场景 - 请求处理管道
void TestRequestPipeline() {
    cout << "\n========== 测试6: 实际应用场景 - 请求处理管道 ==========" << endl;

    struct WebRequest {
        std::string ip;
        std::string user_agent;
        std::string path;
        std::map<std::string, std::string> headers;

        WebRequest(const std::string& i, const std::string& ua, const std::string& p)
            : ip(i), user_agent(ua), path(p) {}
    };

    // 构建请求处理管道
    ChainBuilder<WebRequest> pipeline;

    // 1. IP白名单检查
    pipeline.Add(MakeHandler<WebRequest>([](const WebRequest& req, void*) {
        if (req.ip == "192.168.1.100") {
            cout << "[IP检查] IP在黑名单中，拒绝访问" << endl;
            return HandleResult::kError;
        }
        cout << "[IP检查] IP验证通过" << endl;
        return HandleResult::kNotHandled;
    }));

    // 2. 速率限制
    pipeline.Add(MakeHandler<WebRequest>([](const WebRequest& req, void*) {
        // 模拟速率限制检查
        static int request_count = 0;
        request_count++;
        if (request_count > 100) {
            cout << "[速率限制] 请求过于频繁" << endl;
            return HandleResult::kError;
        }
        cout << "[速率限制] 速率检查通过" << endl;
        return HandleResult::kNotHandled;
    }));

    // 3. 路由处理
    pipeline.Add(MakeHandler<WebRequest>([](const WebRequest& req, void*) {
        if (req.path == "/") {
            cout << "[路由] 处理首页请求" << endl;
            return HandleResult::kHandled;
        } else if (req.path.find("/api/") == 0) {
            cout << "[路由] 处理API请求: " << req.path << endl;
            return HandleResult::kHandled;
        }
        return HandleResult::kNotHandled;
    }));

    auto chain = pipeline.Build();

    // 处理请求
    WebRequest req1("192.168.1.50", "Mozilla/5.0", "/");
    chain->HandleAndPass(req1);

    WebRequest req2("192.168.1.50", "Mozilla/5.0", "/api/users");
    chain->HandleAndPass(req2);

    WebRequest req3("192.168.1.100", "Mozilla/5.0", "/");
    HandleResult result = chain->HandleAndPass(req3);
    if (result == HandleResult::kError) {
        cout << "请求被拒绝" << endl;
    }
}

// 测试7: 混合使用继承和函数式处理者
void TestMixedHandlers() {
    cout << "\n========== 测试7: 混合使用继承和函数式处理者 ==========" << endl;

    struct DataRequest {
        std::string type;
        int id;

        DataRequest(const std::string& t, int i) : type(t), id(i) {}
    };

    // 继承方式
    class CacheHandler : public Handler<DataRequest> {
       public:
        HandleResult Handle(const DataRequest& req, void*) override {
            if (req.type == "cache") {
                cout << "[缓存处理器] 从缓存获取数据 ID: " << req.id << endl;
                return HandleResult::kHandled;
            }
            return HandleResult::kNotHandled;
        }
    };

    // 函数式方式
    auto db_handler = MakeHandler<DataRequest>([](const DataRequest& req, void*) {
        if (req.type == "database") {
            cout << "[数据库处理器] 从数据库查询数据 ID: " << req.id << endl;
            return HandleResult::kHandled;
        }
        return HandleResult::kNotHandled;
    });

    auto file_handler = MakeHandler<DataRequest>([](const DataRequest& req, void*) {
        if (req.type == "file") {
            cout << "[文件处理器] 从文件读取数据 ID: " << req.id << endl;
            return HandleResult::kHandled;
        }
        return HandleResult::kNotHandled;
    });

    // 混合构建链
    auto cache = std::make_shared<CacheHandler>();
    cache->SetNext(db_handler);
    db_handler->SetNext(file_handler);

    // 处理请求
    DataRequest req1("cache", 1);
    cache->HandleAndPass(req1);

    DataRequest req2("database", 2);
    cache->HandleAndPass(req2);

    DataRequest req3("file", 3);
    cache->HandleAndPass(req3);
}

int main() {
    cout << "========================================" << endl;
    cout << "    责任链模式测试程序" << endl;
    cout << "========================================" << endl;

    try {
        TestBasicChain();
        TestFunctionalHandler();
        TestChainBuilder();
        TestChainWithResponse();
        TestChainManager();
        TestRequestPipeline();
        TestMixedHandlers();

        cout << "\n========================================" << endl;
        cout << "    所有测试完成！" << endl;
        cout << "========================================" << endl;
    } catch (const std::exception& e) {
        cout << "测试异常: " << e.what() << endl;
        return 1;
    }

    return 0;
}

