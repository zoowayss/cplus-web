#ifndef PROBLEM_CONTROLLER_H
#define PROBLEM_CONTROLLER_H

#include "base_controller.h"
#include "../http/http_server.h"
#include "../middleware/auth_middleware.h"

class ProblemController : public BaseController {
public:
    ProblemController() = default;
    ~ProblemController() override = default;
    
    // 实现注册路由方法
    void registerRoutes(http::HttpServer* server) override;
    
private:
    // 问题列表处理
    void handleGetProblems(const http::Request& req, http::Response& res);
    
    // 提交代码处理
    void handleSubmitCode(const http::Request& req, http::Response& res);
};

#endif // PROBLEM_CONTROLLER_H 