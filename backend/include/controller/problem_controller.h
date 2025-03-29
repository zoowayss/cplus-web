#ifndef PROBLEM_CONTROLLER_H
#define PROBLEM_CONTROLLER_H

#include "base_controller.h"
#include "../http/http_server.h"
#include "../middleware/auth_middleware.h"
#include "../services/problem_service.h"
#include "../utils/jwt.h"

class ProblemController : public BaseController {
public:
    ProblemController() = default;
    ~ProblemController() override = default;
    
    // 实现注册路由方法
    void registerRoutes(http::HttpServer* server) override;
    
private:
    // 获取题目列表
    void handleGetProblems(const http::Request& req, http::Response& res);
    
    // 获取题目详情
    void handleGetProblemDetail(const http::Request& req, http::Response& res);
    
    // 创建题目
    void handleCreateProblem(const http::Request& req, http::Response& res);
    
    // 更新题目
    void handleUpdateProblem(const http::Request& req, http::Response& res);
    
    // 删除题目
    void handleDeleteProblem(const http::Request& req, http::Response& res);
    
    // 添加测试用例
    void handleAddTestCase(const http::Request& req, http::Response& res);
    
    // 获取测试用例
    void handleGetTestCases(const http::Request& req, http::Response& res);
    
    // 更新测试用例
    void handleUpdateTestCase(const http::Request& req, http::Response& res);
    
    // 删除测试用例
    void handleDeleteTestCase(const http::Request& req, http::Response& res);
    
    // 提交代码处理
    void handleSubmitCode(const http::Request& req, http::Response& res);
    
    // 获取题目的提交记录
    void handleGetProblemSubmissions(const http::Request& req, http::Response& res);
    
    // 获取所有提交记录
    void handleGetAllSubmissions(const http::Request& req, http::Response& res);
    
    // 获取单个提交详情
    void handleGetSubmissionDetail(const http::Request& req, http::Response& res);
    
    // 重新提交代码
    void handleResubmitCode(const http::Request& req, http::Response& res);
    
    // 从路径参数中获取ID
    int getIdFromPath(const std::string& path, const std::string& prefix);
};

#endif // PROBLEM_CONTROLLER_H 