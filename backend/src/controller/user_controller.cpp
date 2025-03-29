#include "../../include/controller/user_controller.h"
#include <iostream>

// 注册所有用户相关路由
void UserController::registerRoutes(http::HttpServer* server) {
    // 用户登录路由
    server->post("/api/user/login", [this](const http::Request& req, http::Response& res) {
        this->handleLogin(req, res);
    });
    
    // 用户注册路由
    server->post("/api/user/register", [this](const http::Request& req, http::Response& res) {
        this->handleRegister(req, res);
    });
    
    // 获取用户信息路由 - 需要身份验证
    server->get("/api/user/profile", middleware::AuthMiddleware::protect([this](const http::Request& req, http::Response& res) {
        this->handleGetProfile(req, res);
    }));
    
    // 更新用户信息路由 - 需要身份验证
    server->put("/api/user/profile", middleware::AuthMiddleware::protect([this](const http::Request& req, http::Response& res) {
        this->handleUpdateProfile(req, res);
    }));
    
    // 更改密码路由 - 需要身份验证
    server->post("/api/user/change_password", middleware::AuthMiddleware::protect([this](const http::Request& req, http::Response& res) {
        this->handleChangePassword(req, res);
    }));
    
    // 验证令牌路由
    server->post("/api/verify_token", [this](const http::Request& req, http::Response& res) {
        this->handleVerifyToken(req, res);
    });
}

// 用户登录处理
void UserController::handleLogin(const http::Request& req, http::Response& res) {
    Json::Value reqJson = parseRequestBody(req.body);
    
    if (!reqJson.isMember("username") || !reqJson.isMember("password")) {
        sendErrorResponse(res, "请提供用户名和密码", 400);
        return;
    }
    
    std::string username = reqJson["username"].asString();
    std::string password = reqJson["password"].asString();
    std::string error_message;
    
    std::string token = UserService::login(username, password, error_message);
    
    if (token.empty()) {
        sendErrorResponse(res, error_message, 401);
    } else {
        Json::Value data;
        data["token"] = token;
        sendSuccessResponse(res, "登录成功", data);
    }
}

// 用户注册处理
void UserController::handleRegister(const http::Request& req, http::Response& res) {
    Json::Value reqJson = parseRequestBody(req.body);
    
    if (!reqJson.isMember("username") || !reqJson.isMember("password") || !reqJson.isMember("email")) {
        sendErrorResponse(res, "请提供用户名、密码和邮箱", 400);
        return;
    }
    
    std::string username = reqJson["username"].asString();
    std::string password = reqJson["password"].asString();
    std::string email = reqJson["email"].asString();
    std::string error_message;
    
    bool success = UserService::registerUser(username, password, email, error_message);
    
    if (success) {
        sendSuccessResponse(res, "注册成功，请登录");
    } else {
        sendErrorResponse(res, error_message, 400);
    }
}

// 获取用户信息处理
void UserController::handleGetProfile(const http::Request& req, http::Response& res) {
    // 从请求头获取令牌
    std::string auth_header = req.get_header("Authorization");
    std::string token = auth_header.substr(7);
    
    // 获取用户ID
    int user_id = JWT::getUserIdFromToken(token);
    
    if (user_id <= 0) {
        sendErrorResponse(res, "无效的用户ID", 401);
        return;
    }
    
    // 获取用户信息
    User user = UserService::getUserInfo(user_id);
    
    if (user.getId() == 0) {
        sendErrorResponse(res, "用户不存在", 404);
        return;
    }
    
    // 将用户信息添加到响应中
    Json::Value userJson = parseRequestBody(user.toJson());
    Json::Value data;
    data["user"] = userJson;
    
    sendSuccessResponse(res, "获取用户信息成功", data);
}

// 更新用户信息处理
void UserController::handleUpdateProfile(const http::Request& req, http::Response& res) {
    // 从请求头获取令牌
    std::string auth_header = req.get_header("Authorization");
    std::string token = auth_header.substr(7);
    
    // 获取用户ID
    int user_id = JWT::getUserIdFromToken(token);
    
    if (user_id <= 0) {
        sendErrorResponse(res, "无效的用户ID", 401);
        return;
    }
    
    // 解析请求体
    Json::Value reqJson = parseRequestBody(req.body);
    
    // 构建用户数据映射
    std::map<std::string, std::string> user_data;
    
    if (reqJson.isMember("username")) user_data["username"] = reqJson["username"].asString();
    if (reqJson.isMember("email")) user_data["email"] = reqJson["email"].asString();
    if (reqJson.isMember("avatar")) user_data["avatar"] = reqJson["avatar"].asString();
    
    // 更新用户信息
    std::string error_message;
    bool success = UserService::updateUserInfo(user_id, user_data, error_message);
    
    if (success) {
        // 获取更新后的用户信息
        User user = UserService::getUserInfo(user_id);
        
        // 将用户信息添加到响应中
        Json::Value userJson = parseRequestBody(user.toJson());
        Json::Value data;
        data["user"] = userJson;
        
        sendSuccessResponse(res, "用户信息更新成功", data);
    } else {
        sendErrorResponse(res, error_message, 400);
    }
}

// 修改密码处理
void UserController::handleChangePassword(const http::Request& req, http::Response& res) {
    // 从请求头获取令牌
    std::string auth_header = req.get_header("Authorization");
    std::string token = auth_header.substr(7);
    
    // 获取用户ID
    int user_id = JWT::getUserIdFromToken(token);
    
    if (user_id <= 0) {
        sendErrorResponse(res, "无效的用户ID", 401);
        return;
    }
    
    // 解析请求体
    Json::Value reqJson = parseRequestBody(req.body);
    
    if (!reqJson.isMember("old_password") || !reqJson.isMember("new_password")) {
        sendErrorResponse(res, "请提供旧密码和新密码", 400);
        return;
    }
    
    std::string old_password = reqJson["old_password"].asString();
    std::string new_password = reqJson["new_password"].asString();
    
    // 更改密码
    std::string error_message;
    bool success = UserService::changePassword(user_id, old_password, new_password, error_message);
    
    if (success) {
        sendSuccessResponse(res, "密码更改成功");
    } else {
        sendErrorResponse(res, error_message, 400);
    }
}

// 验证令牌处理
void UserController::handleVerifyToken(const http::Request& req, http::Response& res) {
    // 从请求头获取令牌
    std::string auth_header = req.get_header("Authorization");
    
    if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
        sendErrorResponse(res, "未提供有效的认证令牌", 401);
        return;
    }
    
    // 提取令牌
    std::string token = auth_header.substr(7);
    
    // 验证令牌
    std::map<std::string, std::string> payload;
    if (!JWT::verifyToken(token, payload)) {
        sendErrorResponse(res, "认证令牌无效或已过期", 401);
        return;
    }
    
    // 获取用户ID和角色
    int user_id = JWT::getUserIdFromToken(token);
    int role = JWT::getUserRoleFromToken(token);
    
    // 获取用户信息
    User user = UserService::getUserInfo(user_id);
    
    if (user.getId() == 0) {
        sendErrorResponse(res, "用户不存在", 404);
        return;
    }
    
    // 返回验证结果
    Json::Value data;
    data["user_id"] = user_id;
    data["role"] = role;
    data["username"] = user.getUsername();
    
    sendSuccessResponse(res, "令牌有效", data);
} 