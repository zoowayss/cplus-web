#include "../../include/controller/system_controller.h"

void SystemController::registerRoutes(http::HttpServer* server) {
    // 根路径 - 健康检查
    server->get("/api", [this](const http::Request& req, http::Response& res) {
        this->handleHealthCheck(req, res);
    });
    
    // 测试数据库连接路由
    server->get("/api/db_test", [this](const http::Request& req, http::Response& res) {
        this->handleDbTest(req, res);
    });
}

// 健康检查处理
void SystemController::handleHealthCheck(const http::Request& req, http::Response& res) {
    sendSuccessResponse(res, "C++ 在线评测系统API服务正常运行");
}

// 数据库测试处理
void SystemController::handleDbTest(const http::Request& req, http::Response& res) {
    Database* db = Database::getInstance();
    MYSQL_RES* result = db->executeQuery("SELECT 1 as test");
    
    if (result) {
        mysql_free_result(result);
        sendSuccessResponse(res, "数据库连接正常");
    } else {
        sendErrorResponse(res, "数据库连接异常", 500);
    }
} 