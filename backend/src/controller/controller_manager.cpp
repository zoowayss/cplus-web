#include "../../include/controller/controller_manager.h"

ControllerManager::ControllerManager() {
    // 初始化所有控制器 - 使用C++11兼容的方式创建unique_ptr
    controllers.push_back(std::unique_ptr<UserController>(new UserController()));
    controllers.push_back(std::unique_ptr<SystemController>(new SystemController()));
    controllers.push_back(std::unique_ptr<ProblemController>(new ProblemController()));
    controllers.push_back(std::unique_ptr<DiscussionController>(new DiscussionController()));
}

ControllerManager::~ControllerManager() = default;

// 注册所有控制器的路由
void ControllerManager::registerAllRoutes(http::HttpServer* server) {
    for (auto& controller : controllers) {
        controller->registerRoutes(server);
    }
} 