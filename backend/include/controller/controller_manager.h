#ifndef CONTROLLER_MANAGER_H
#define CONTROLLER_MANAGER_H

#include "../http/http_server.h"
#include "user_controller.h"
#include "system_controller.h"
#include "problem_controller.h"
#include "discussion_controller.h"
#include <vector>
#include <memory>

class ControllerManager {
public:
    ControllerManager();
    ~ControllerManager();
    
    // 注册所有控制器的路由
    void registerAllRoutes(http::HttpServer* server);
    
private:
    // 控制器列表
    std::vector<std::unique_ptr<BaseController>> controllers;
};

#endif // CONTROLLER_MANAGER_H 