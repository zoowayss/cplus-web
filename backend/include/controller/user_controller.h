#ifndef USER_CONTROLLER_H
#define USER_CONTROLLER_H

#include "base_controller.h"
#include "../services/user_service.h"
#include "../services/submission_service.h"
#include "../http/http_server.h"
#include "../middleware/auth_middleware.h"

class UserController : public BaseController {
public:
    UserController() = default;
    ~UserController() override = default;
    
    // 实现注册路由方法
    void registerRoutes(http::HttpServer* server) override;
    
private:
    // 用户登录处理
    void handleLogin(const http::Request& req, http::Response& res);
    
    // 用户注册处理
    void handleRegister(const http::Request& req, http::Response& res);
    
    // 获取用户信息处理
    void handleGetProfile(const http::Request& req, http::Response& res);
    
    // 更新用户信息处理
    void handleUpdateProfile(const http::Request& req, http::Response& res);
    
    // 修改密码处理
    void handleChangePassword(const http::Request& req, http::Response& res);
    
    // 验证令牌处理
    void handleVerifyToken(const http::Request& req, http::Response& res);
    
    // 获取用户排行榜处理
    void handleGetLeaderboard(const http::Request& req, http::Response& res);
    
    // 获取用户题目状态处理
    void handleGetProblemStatus(const http::Request& req, http::Response& res);
};

#endif // USER_CONTROLLER_H 