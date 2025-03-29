#include "../../include/controller/controller_manager.h"

ControllerManager::ControllerManager() {
    // 初始化所有控制器
    controllers.push_back(std::make_unique<UserController>());
    controllers.push_back(std::make_unique<SystemController>());
    controllers.push_back(std::make_unique<ProblemController>());
}

ControllerManager::~ControllerManager() = default;

// 注册所有控制器的路由
void ControllerManager::registerAllRoutes(http::HttpServer* server) {
    for (auto& controller : controllers) {
        controller->registerRoutes(server);
    }
} 