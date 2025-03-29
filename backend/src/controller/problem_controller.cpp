#include "../../include/controller/problem_controller.h"

void ProblemController::registerRoutes(http::HttpServer* server) {
    // 问题列表路由
    server->get("/api/problems", [this](const http::Request& req, http::Response& res) {
        this->handleGetProblems(req, res);
    });
    
    // 提交代码路由
    server->post("/api/submissions", middleware::AuthMiddleware::protect([this](const http::Request& req, http::Response& res) {
        this->handleSubmitCode(req, res);
    }));
}

// 问题列表处理
void ProblemController::handleGetProblems(const http::Request& req, http::Response& res) {
    Json::Value problems(Json::arrayValue);
    Json::Value data;
    data["problems"] = problems;
    
    sendSuccessResponse(res, "题目列表功能未实现", data);
}

// 提交代码处理
void ProblemController::handleSubmitCode(const http::Request& req, http::Response& res) {
    Json::Value data;
    data["submission_id"] = 1;
    
    sendSuccessResponse(res, "代码提交功能未实现", data);
} 