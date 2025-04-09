#include "../../include/controller/user_controller.h"
#include <iostream>
#include <sstream>
#include <string>

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
    
    // 获取用户排行榜路由
    server->get("/api/leaderboard", middleware::AuthMiddleware::protect([this](const http::Request& req, http::Response& res) {
        this->handleGetLeaderboard(req, res);
    }));
    
    // 获取用户题目状态路由
    server->get("/api/user/problem-status", middleware::AuthMiddleware::protect([this](const http::Request& req, http::Response& res) {
        this->handleGetProblemStatus(req, res);
    }));
    
    // ===== 管理员用户管理相关路由 =====
    
    // 获取所有用户列表路由 - 需要管理员权限
    server->get("/api/admin/users", middleware::AuthMiddleware::protectWithRole([this](const http::Request& req, http::Response& res) {
        this->handleGetAllUsers(req, res);
    }, static_cast<int>(UserRole::ADMIN)));
    
    // 创建用户路由 - 需要管理员权限
    server->post("/api/admin/users", middleware::AuthMiddleware::protectWithRole([this](const http::Request& req, http::Response& res) {
        this->handleCreateUser(req, res);
    }, static_cast<int>(UserRole::ADMIN)));
    
    // 更新用户信息路由 - 需要管理员权限
    server->put("/api/admin/users/:id", middleware::AuthMiddleware::protectWithRole([this](const http::Request& req, http::Response& res) {
        this->handleUpdateUser(req, res);
    }, static_cast<int>(UserRole::ADMIN)));
    
    // 删除用户路由 - 需要管理员权限
    server->del("/api/admin/users/:id", middleware::AuthMiddleware::protectWithRole([this](const http::Request& req, http::Response& res) {
        this->handleDeleteUser(req, res);
    }, static_cast<int>(UserRole::ADMIN)));
    
    // 更改用户角色路由 - 需要管理员权限
    server->post("/api/admin/users/:id/role", middleware::AuthMiddleware::protectWithRole([this](const http::Request& req, http::Response& res) {
        this->handleChangeUserRole(req, res);
    }, static_cast<int>(UserRole::ADMIN)));
    
    // 更改用户状态路由 - 需要管理员权限
    server->post("/api/admin/users/:id/status", middleware::AuthMiddleware::protectWithRole([this](const http::Request& req, http::Response& res) {
        this->handleChangeUserStatus(req, res);
    }, static_cast<int>(UserRole::ADMIN)));
    
    // 重置用户密码路由 - 需要管理员权限
    server->post("/api/admin/users/:id/reset_password", middleware::AuthMiddleware::protectWithRole([this](const http::Request& req, http::Response& res) {
        this->handleResetUserPassword(req, res);
    }, static_cast<int>(UserRole::ADMIN)));
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
    
    bool success = UserService::registerUser(username, email, password, error_message);
    
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

// 获取用户排行榜处理
void UserController::handleGetLeaderboard(const http::Request& req, http::Response& res) {
    // 获取请求参数
    int offset = 0;
    int limit = 10;
    std::string time_range = "all";

    // 解析URL查询参数
    std::string path = req.path;
    size_t pos = path.find('?');
    if (pos != std::string::npos) {
        std::string query = path.substr(pos + 1);
        std::istringstream ss(query);
        std::string param;
        
        while (std::getline(ss, param, '&')) {
            size_t equal_pos = param.find('=');
            if (equal_pos != std::string::npos) {
                std::string key = param.substr(0, equal_pos);
                std::string value = param.substr(equal_pos + 1);
                
                try {
                    if (key == "offset" && !value.empty()) {
                        offset = std::stoi(value);
                    } else if (key == "limit" && !value.empty()) {
                        limit = std::stoi(value);
                    } else if (key == "time_range" && !value.empty()) {
                        time_range = value;
                    }
                } catch (...) {
                    // 忽略转换错误，使用默认值
                }
            }
        }
    }
    
    // 从请求头获取令牌
    std::string auth_header = req.get_header("Authorization");
    std::string token = auth_header.substr(7);
    
    // 获取当前用户ID
    int current_user_id = JWT::getUserIdFromToken(token);
    
    // 获取排行榜数据
    Json::Value leaderboard_data;
    int total = 0;
    std::string error_message;
    
    bool success = UserService::getLeaderboard(offset, limit, time_range, current_user_id, leaderboard_data, total, error_message);
    
    if (success) {
        Json::Value data;
        data["leaderboard"] = leaderboard_data;
        data["total"] = total;
        
        sendSuccessResponse(res, "获取排行榜成功", data);
    } else {
        sendErrorResponse(res, error_message, 400);
    }
}

// 获取用户题目状态处理
void UserController::handleGetProblemStatus(const http::Request& req, http::Response& res) {
    // 从请求头获取令牌
    std::string auth_header = req.get_header("Authorization");
    std::string token = auth_header.substr(7);
    
    // 获取当前用户ID
    int user_id = JWT::getUserIdFromToken(token);
    
    if (user_id <= 0) {
        sendErrorResponse(res, "无效的用户ID", 401);
        return;
    }
    
    // 获取用户题目状态
    Json::Value status_data;
    std::string error_message;
    
    // 假设我们有一个从SubmissionService中获取用户题目状态的方法
    bool success = SubmissionService::getUserProblemStatus(user_id, status_data, error_message);
    
    if (success) {
        sendSuccessResponse(res, "获取题目状态成功", status_data);
    } else {
        sendErrorResponse(res, error_message, 400);
    }
}

// ===== 管理员用户管理相关处理函数 =====

// 获取所有用户列表
void UserController::handleGetAllUsers(const http::Request& req, http::Response& res) {
    try {
        // 解析URL查询参数
        int offset = 0;
        int limit = 10;
        std::string search_term = "";
        int role_filter = -1;
        int status_filter = -1;
        
        // 从请求参数中获取值
        if (req.has_param("offset")) {
            try {
                offset = std::stoi(req.get_param("offset"));
            } catch (...) {
                // 使用默认值
            }
        }
        
        if (req.has_param("limit")) {
            try {
                limit = std::stoi(req.get_param("limit"));
            } catch (...) {
                // 使用默认值
            }
        }
        
        if (req.has_param("search")) {
            search_term = req.get_param("search");
        }
        
        if (req.has_param("role")) {
            try {
                std::string role_str = req.get_param("role");
                if (role_str == "0") {
                    role_filter = 0;
                } else if (!role_str.empty()) {
                    role_filter = std::stoi(role_str);
                }
            } catch (...) {
                // 使用默认值
            }
        }
        
        if (req.has_param("status")) {
            try {
                std::string status_str = req.get_param("status");
                if (status_str == "0") {
                    status_filter = 0;
                } else if (!status_str.empty()) {
                    status_filter = std::stoi(status_str);
                }
            } catch (...) {
                // 使用默认值
            }
        }
        
        // 输出日志，便于调试
        std::cout << "用户列表请求参数: offset=" << offset << ", limit=" << limit 
                << ", search=" << search_term << ", role=" << role_filter 
                << ", status=" << status_filter << std::endl;
        
        // 获取用户列表
        Json::Value users_data;
        int total = 0;
        std::string error_message;
        
        bool success = UserService::getAllUsers(offset, limit, search_term, 
                                            role_filter, status_filter, 
                                            users_data, total, error_message);
        
        if (success) {
            Json::Value data;
            data["users"] = users_data;
            data["total"] = total;
            data["offset"] = offset;
            data["limit"] = limit;
            
            sendSuccessResponse(res, "获取用户列表成功", data);
        } else {
            sendErrorResponse(res, error_message, 400);
        }
    } catch (const std::exception& e) {
        // 捕获所有异常，避免500错误
        std::cerr << "获取用户列表出错: " << e.what() << std::endl;
        sendErrorResponse(res, "服务器内部错误: " + std::string(e.what()), 500);
    } catch (...) {
        std::cerr << "获取用户列表出现未知错误" << std::endl;
        sendErrorResponse(res, "服务器内部错误", 500);
    }
}

// 创建用户（管理员）
void UserController::handleCreateUser(const http::Request& req, http::Response& res) {
    Json::Value reqJson = parseRequestBody(req.body);
    
    if (!reqJson.isMember("username") || !reqJson.isMember("password") || 
        !reqJson.isMember("email") || !reqJson.isMember("role")) {
        sendErrorResponse(res, "请提供用户名、密码、邮箱和角色", 400);
        return;
    }
    
    std::string username = reqJson["username"].asString();
    std::string password = reqJson["password"].asString();
    std::string email = reqJson["email"].asString();
    int role = reqJson["role"].asInt();
    std::string error_message;
    
    bool success = UserService::createUserByAdmin(username, password, email, role, error_message);
    
    if (success) {
        sendSuccessResponse(res, "创建用户成功");
    } else {
        sendErrorResponse(res, error_message, 400);
    }
}

// 更新用户信息（管理员）
void UserController::handleUpdateUser(const http::Request& req, http::Response& res) {
    // 从URL路径获取用户ID
    std::string path = req.path;
    size_t last_slash_pos = path.find_last_of('/');
    
    if (last_slash_pos == std::string::npos || last_slash_pos == path.length() - 1) {
        sendErrorResponse(res, "无效的用户ID", 400);
        return;
    }
    
    std::string id_str = path.substr(last_slash_pos + 1);
    // 从ID字符串中移除查询参数（如果有）
    size_t query_pos = id_str.find('?');
    if (query_pos != std::string::npos) {
        id_str = id_str.substr(0, query_pos);
    }
    
    int user_id;
    try {
        user_id = std::stoi(id_str);
    } catch (...) {
        sendErrorResponse(res, "无效的用户ID", 400);
        return;
    }
    
    // 解析请求体
    Json::Value reqJson = parseRequestBody(req.body);
    std::string error_message;
    
    // 调用更新用户服务
    bool success = UserService::updateUserByAdmin(user_id, reqJson, error_message);
    
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

// 删除用户（管理员）
void UserController::handleDeleteUser(const http::Request& req, http::Response& res) {
    // 从URL路径获取用户ID
    std::string path = req.path;
    size_t last_slash_pos = path.find_last_of('/');
    
    if (last_slash_pos == std::string::npos || last_slash_pos == path.length() - 1) {
        sendErrorResponse(res, "无效的用户ID", 400);
        return;
    }
    
    std::string id_str = path.substr(last_slash_pos + 1);
    // 从ID字符串中移除查询参数（如果有）
    size_t query_pos = id_str.find('?');
    if (query_pos != std::string::npos) {
        id_str = id_str.substr(0, query_pos);
    }
    
    int user_id;
    try {
        user_id = std::stoi(id_str);
    } catch (...) {
        sendErrorResponse(res, "无效的用户ID", 400);
        return;
    }
    
    // 获取当前操作用户ID以确保不删除自己
    std::string auth_header = req.get_header("Authorization");
    std::string token = auth_header.substr(7);
    int current_user_id = JWT::getUserIdFromToken(token);
    
    if (user_id == current_user_id) {
        sendErrorResponse(res, "不能删除自己的账户", 400);
        return;
    }
    
    // 调用删除用户服务
    std::string error_message;
    bool success = UserService::deleteUser(user_id, error_message);
    
    if (success) {
        sendSuccessResponse(res, "用户删除成功");
    } else {
        sendErrorResponse(res, error_message, 400);
    }
}

// 更改用户角色（管理员）
void UserController::handleChangeUserRole(const http::Request& req, http::Response& res) {
    // 从URL路径获取用户ID
    std::string path = req.path;
    size_t last_slash_pos = path.find_last_of('/');
    
    if (last_slash_pos == std::string::npos || last_slash_pos == path.length() - 1) {
        sendErrorResponse(res, "无效的用户ID", 400);
        return;
    }
    
    std::string id_part = path.substr(0, last_slash_pos);
    size_t user_id_pos = id_part.find_last_of('/');
    
    if (user_id_pos == std::string::npos || user_id_pos == id_part.length() - 1) {
        sendErrorResponse(res, "无效的用户ID", 400);
        return;
    }
    
    std::string id_str = id_part.substr(user_id_pos + 1);
    int user_id;
    try {
        user_id = std::stoi(id_str);
    } catch (...) {
        sendErrorResponse(res, "无效的用户ID", 400);
        return;
    }
    
    // 获取当前操作用户ID以确保不更改自己的角色
    std::string auth_header = req.get_header("Authorization");
    std::string token = auth_header.substr(7);
    int current_user_id = JWT::getUserIdFromToken(token);
    
    if (user_id == current_user_id) {
        sendErrorResponse(res, "不能更改自己的角色", 400);
        return;
    }
    
    // 解析请求体
    Json::Value reqJson = parseRequestBody(req.body);
    
    if (!reqJson.isMember("role")) {
        sendErrorResponse(res, "请提供角色值", 400);
        return;
    }
    
    int role = reqJson["role"].asInt();
    if (role < 0 || role > 2) {
        sendErrorResponse(res, "无效的角色值", 400);
        return;
    }
    
    // 调用更改用户角色服务
    std::string error_message;
    bool success = UserService::changeUserRole(user_id, static_cast<UserRole>(role), error_message);
    
    if (success) {
        sendSuccessResponse(res, "用户角色更改成功");
    } else {
        sendErrorResponse(res, error_message, 400);
    }
}

// 更改用户状态（管理员）
void UserController::handleChangeUserStatus(const http::Request& req, http::Response& res) {
    // 从URL路径获取用户ID
    std::string path = req.path;
    size_t last_slash_pos = path.find_last_of('/');
    
    if (last_slash_pos == std::string::npos || last_slash_pos == path.length() - 1) {
        sendErrorResponse(res, "无效的用户ID", 400);
        return;
    }
    
    std::string id_part = path.substr(0, last_slash_pos);
    size_t user_id_pos = id_part.find_last_of('/');
    
    if (user_id_pos == std::string::npos || user_id_pos == id_part.length() - 1) {
        sendErrorResponse(res, "无效的用户ID", 400);
        return;
    }
    
    std::string id_str = id_part.substr(user_id_pos + 1);
    int user_id;
    try {
        user_id = std::stoi(id_str);
    } catch (...) {
        sendErrorResponse(res, "无效的用户ID", 400);
        return;
    }
    
    // 获取当前操作用户ID以确保不更改自己的状态
    std::string auth_header = req.get_header("Authorization");
    std::string token = auth_header.substr(7);
    int current_user_id = JWT::getUserIdFromToken(token);
    
    if (user_id == current_user_id) {
        sendErrorResponse(res, "不能更改自己的状态", 400);
        return;
    }
    
    // 解析请求体
    Json::Value reqJson = parseRequestBody(req.body);
    
    if (!reqJson.isMember("status")) {
        sendErrorResponse(res, "请提供状态值", 400);
        return;
    }
    
    int status = reqJson["status"].asInt();
    if (status < 0 || status > 2) {
        sendErrorResponse(res, "无效的状态值", 400);
        return;
    }
    
    // 调用更改用户状态服务
    std::string error_message;
    bool success = UserService::changeUserStatus(user_id, static_cast<UserStatus>(status), error_message);
    
    if (success) {
        sendSuccessResponse(res, "用户状态更改成功");
    } else {
        sendErrorResponse(res, error_message, 400);
    }
}

// 重置用户密码（管理员）
void UserController::handleResetUserPassword(const http::Request& req, http::Response& res) {
    // 从URL路径获取用户ID
    std::string path = req.path;
    size_t last_slash_pos = path.find_last_of('/');
    
    if (last_slash_pos == std::string::npos || last_slash_pos == path.length() - 1) {
        sendErrorResponse(res, "无效的用户ID", 400);
        return;
    }
    
    std::string id_part = path.substr(0, last_slash_pos);
    size_t user_id_pos = id_part.find_last_of('/');
    
    if (user_id_pos == std::string::npos || user_id_pos == id_part.length() - 1) {
        sendErrorResponse(res, "无效的用户ID", 400);
        return;
    }
    
    std::string id_str = id_part.substr(user_id_pos + 1);
    int user_id;
    try {
        user_id = std::stoi(id_str);
    } catch (...) {
        sendErrorResponse(res, "无效的用户ID", 400);
        return;
    }
    
    // 解析请求体
    Json::Value reqJson = parseRequestBody(req.body);
    
    if (!reqJson.isMember("new_password")) {
        sendErrorResponse(res, "请提供新密码", 400);
        return;
    }
    
    std::string new_password = reqJson["new_password"].asString();
    
    // 调用重置密码服务
    std::string error_message;
    bool success = UserService::resetUserPassword(user_id, new_password, error_message);
    
    if (success) {
        sendSuccessResponse(res, "用户密码重置成功");
    } else {
        sendErrorResponse(res, error_message, 400);
    }
} 