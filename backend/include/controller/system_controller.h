#ifndef SYSTEM_CONTROLLER_H
#define SYSTEM_CONTROLLER_H

#include "base_controller.h"
#include "../http/http_server.h"
#include "../database/database.h"

class SystemController : public BaseController {
public:
    SystemController() = default;
    ~SystemController() override = default;
    
    // 实现注册路由方法
    void registerRoutes(http::HttpServer* server) override;
    
private:
    // 健康检查处理
    void handleHealthCheck(const http::Request& req, http::Response& res);
    
    // 数据库测试处理
    void handleDbTest(const http::Request& req, http::Response& res);
};

#endif // SYSTEM_CONTROLLER_H 