#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <csignal>
#include "../include/database/database.h"
#include "../include/http/http_server.h"
#include "../include/utils/jwt.h"
#include "../include/services/user_service.h"
#include "../include/middleware/auth_middleware.h"
#include <json/json.h>

// 全局HTTP服务器实例
http::HttpServer* server = nullptr;

// 信号处理函数，用于优雅地关闭服务器
void signal_handler(int signal) {
    std::cout << "接收到信号 " << signal << "，正在关闭服务器..." << std::endl;
    if (server) {
        server->stop();
    }
    exit(0);
}

// 从请求体解析JSON
Json::Value parseRequestBody(const std::string& body) {
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;
    std::istringstream iss(body);
    
    if (!Json::parseFromStream(builder, iss, &root, &errors)) {
        return Json::Value();
    }
    
    return root;
}

int main(int argc, char** argv) {
    std::cout << "启动在线评测系统后端..." << std::endl;
    
    // 注册信号处理器，以便正确处理Ctrl+C等信号
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // 初始化数据库连接
    Database* db = Database::getInstance();
    bool connected = db->initialize(
        "47.109.39.201",                  // 主机名
        "root",                           // 用户名
        "f4N:1!GRbb]UtdGeP:rP",           // 密码
        "cplus",                          // 数据库名
        3306                              // 端口
    );
    
    if (!connected) {
        std::cerr << "无法连接到数据库，请检查配置" << std::endl;
        return 1;
    }
    
    std::cout << "数据库连接成功" << std::endl;
    
    // 创建HTTP服务器，监听8080端口
    server = new http::HttpServer(8080);
    
    // 设置API路由
    
    // 根路径 - 健康检查
    server->get("/api", [](const http::Request& req, http::Response& res) {
        res.json("{\"status\": \"ok\", \"message\": \"C++ 在线评测系统API服务正常运行\"}");
    });
    
    // 测试数据库连接路由
    server->get("/api/db_test", [db](const http::Request& req, http::Response& res) {
        MYSQL_RES* result = db->executeQuery("SELECT 1 as test");
        if (result) {
            mysql_free_result(result);
            res.json("{\"status\": \"ok\", \"message\": \"数据库连接正常\"}");
        } else {
            res.status_code = 500;
            res.status_message = "Internal Server Error";
            res.json("{\"status\": \"error\", \"message\": \"数据库连接异常\"}");
        }
    });
    
    // 用户登录路由
    server->post("/api/login", [](const http::Request& req, http::Response& res) {
        Json::Value reqJson = parseRequestBody(req.body);
        
        if (!reqJson.isMember("username") || !reqJson.isMember("password")) {
            res.status_code = 400;
            res.status_message = "Bad Request";
            res.json("{\"status\": \"error\", \"message\": \"请提供用户名和密码\"}");
            return;
        }
        
        std::string username = reqJson["username"].asString();
        std::string password = reqJson["password"].asString();
        std::string error_message;
        
        std::string token = UserService::login(username, password, error_message);
        
        if (token.empty()) {
            res.status_code = 401;
            res.status_message = "Unauthorized";
            
            Json::Value response;
            response["status"] = "error";
            response["message"] = error_message;
            
            Json::StreamWriterBuilder writer;
            res.json(Json::writeString(writer, response));
        } else {
            Json::Value response;
            response["status"] = "ok";
            response["message"] = "登录成功";
            response["token"] = token;
            
            Json::StreamWriterBuilder writer;
            res.json(Json::writeString(writer, response));
        }
    });
    
    // 用户注册路由
    server->post("/api/register", [](const http::Request& req, http::Response& res) {
        Json::Value reqJson = parseRequestBody(req.body);
        
        if (!reqJson.isMember("username") || !reqJson.isMember("password") || !reqJson.isMember("email")) {
            res.status_code = 400;
            res.status_message = "Bad Request";
            res.json("{\"status\": \"error\", \"message\": \"请提供用户名、密码和邮箱\"}");
            return;
        }
        
        std::string username = reqJson["username"].asString();
        std::string password = reqJson["password"].asString();
        std::string email = reqJson["email"].asString();
        std::string error_message;
        
        bool success = UserService::registerUser(username, password, email, error_message);
        
        if (success) {
            res.json("{\"status\": \"ok\", \"message\": \"注册成功，请登录\"}");
        } else {
            res.status_code = 400;
            res.status_message = "Bad Request";
            
            Json::Value response;
            response["status"] = "error";
            response["message"] = error_message;
            
            Json::StreamWriterBuilder writer;
            res.json(Json::writeString(writer, response));
        }
    });
    
    // 获取用户信息路由 - 需要身份验证
    server->get("/api/user/profile", middleware::AuthMiddleware::protect([](const http::Request& req, http::Response& res) {
        // 从请求头获取令牌
        std::string auth_header = req.get_header("Authorization");
        std::string token = auth_header.substr(7);
        
        // 获取用户ID
        int user_id = JWT::getUserIdFromToken(token);
        
        if (user_id <= 0) {
            res.status_code = 401;
            res.status_message = "Unauthorized";
            res.json("{\"status\": \"error\", \"message\": \"无效的用户ID\"}");
            return;
        }
        
        // 获取用户信息
        User user = UserService::getUserInfo(user_id);
        
        if (user.getId() == 0) {
            res.status_code = 404;
            res.status_message = "Not Found";
            res.json("{\"status\": \"error\", \"message\": \"用户不存在\"}");
            return;
        }
        
        // 返回用户信息（不包含敏感信息）
        Json::Value response;
        response["status"] = "ok";
        response["message"] = "获取用户信息成功";
        
        // 将用户信息添加到响应中
        Json::Value userJson = parseRequestBody(user.toJson());
        response["user"] = userJson;
        
        Json::StreamWriterBuilder writer;
        res.json(Json::writeString(writer, response));
    }));
    
    // 更新用户信息路由 - 需要身份验证
    server->put("/api/user/profile", middleware::AuthMiddleware::protect([](const http::Request& req, http::Response& res) {
        // 从请求头获取令牌
        std::string auth_header = req.get_header("Authorization");
        std::string token = auth_header.substr(7);
        
        // 获取用户ID
        int user_id = JWT::getUserIdFromToken(token);
        
        if (user_id <= 0) {
            res.status_code = 401;
            res.status_message = "Unauthorized";
            res.json("{\"status\": \"error\", \"message\": \"无效的用户ID\"}");
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
            
            Json::Value response;
            response["status"] = "ok";
            response["message"] = "用户信息更新成功";
            
            // 将用户信息添加到响应中
            Json::Value userJson = parseRequestBody(user.toJson());
            response["user"] = userJson;
            
            Json::StreamWriterBuilder writer;
            res.json(Json::writeString(writer, response));
        } else {
            res.status_code = 400;
            res.status_message = "Bad Request";
            
            Json::Value response;
            response["status"] = "error";
            response["message"] = error_message;
            
            Json::StreamWriterBuilder writer;
            res.json(Json::writeString(writer, response));
        }
    }));
    
    // 更改密码路由 - 需要身份验证
    server->post("/api/user/change_password", middleware::AuthMiddleware::protect([](const http::Request& req, http::Response& res) {
        // 从请求头获取令牌
        std::string auth_header = req.get_header("Authorization");
        std::string token = auth_header.substr(7);
        
        // 获取用户ID
        int user_id = JWT::getUserIdFromToken(token);
        
        if (user_id <= 0) {
            res.status_code = 401;
            res.status_message = "Unauthorized";
            res.json("{\"status\": \"error\", \"message\": \"无效的用户ID\"}");
            return;
        }
        
        // 解析请求体
        Json::Value reqJson = parseRequestBody(req.body);
        
        if (!reqJson.isMember("old_password") || !reqJson.isMember("new_password")) {
            res.status_code = 400;
            res.status_message = "Bad Request";
            res.json("{\"status\": \"error\", \"message\": \"请提供旧密码和新密码\"}");
            return;
        }
        
        std::string old_password = reqJson["old_password"].asString();
        std::string new_password = reqJson["new_password"].asString();
        
        // 更改密码
        std::string error_message;
        bool success = UserService::changePassword(user_id, old_password, new_password, error_message);
        
        if (success) {
            res.json("{\"status\": \"ok\", \"message\": \"密码更改成功\"}");
        } else {
            res.status_code = 400;
            res.status_message = "Bad Request";
            
            Json::Value response;
            response["status"] = "error";
            response["message"] = error_message;
            
            Json::StreamWriterBuilder writer;
            res.json(Json::writeString(writer, response));
        }
    }));
    
    // 问题列表路由
    server->get("/api/problems", [](const http::Request& req, http::Response& res) {
        // 返回题目列表
        res.json("{\"status\": \"ok\", \"message\": \"题目列表功能未实现\", \"problems\": []}");
    });
    
    // 提交代码路由
    server->post("/api/submissions", middleware::AuthMiddleware::protect([](const http::Request& req, http::Response& res) {
        // 这里应该实现代码提交逻辑
        res.json("{\"status\": \"ok\", \"message\": \"代码提交功能未实现\", \"submission_id\": 1}");
    }));
    
    // 验证令牌路由
    server->post("/api/verify_token", [](const http::Request& req, http::Response& res) {
        // 从请求头获取令牌
        std::string auth_header = req.get_header("Authorization");
        
        if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
            res.status_code = 401;
            res.status_message = "Unauthorized";
            res.json("{\"status\": \"error\", \"message\": \"未提供有效的认证令牌\"}");
            return;
        }
        
        // 提取令牌
        std::string token = auth_header.substr(7);
        
        // 验证令牌
        std::map<std::string, std::string> payload;
        if (!JWT::verifyToken(token, payload)) {
            res.status_code = 401;
            res.status_message = "Unauthorized";
            res.json("{\"status\": \"error\", \"message\": \"认证令牌无效或已过期\"}");
            return;
        }
        
        // 获取用户ID和角色
        int user_id = JWT::getUserIdFromToken(token);
        int role = JWT::getUserRoleFromToken(token);
        
        // 获取用户信息
        User user = UserService::getUserInfo(user_id);
        
        if (user.getId() == 0) {
            res.status_code = 404;
            res.status_message = "Not Found";
            res.json("{\"status\": \"error\", \"message\": \"用户不存在\"}");
            return;
        }
        
        // 返回验证结果
        Json::Value response;
        response["status"] = "ok";
        response["message"] = "令牌有效";
        response["user_id"] = user_id;
        response["role"] = role;
        response["username"] = user.getUsername();
        
        Json::StreamWriterBuilder writer;
        res.json(Json::writeString(writer, response));
    });
    
    // 创建数据库表（如果不存在）
    std::string create_users_table = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INT AUTO_INCREMENT PRIMARY KEY,"
        "username VARCHAR(50) UNIQUE NOT NULL,"
        "email VARCHAR(100) UNIQUE NOT NULL,"
        "password_hash VARCHAR(100) NOT NULL,"
        "salt VARCHAR(32) NOT NULL,"
        "avatar VARCHAR(255) DEFAULT '',"
        "role INT DEFAULT 0,"
        "status INT DEFAULT 0,"
        "created_at BIGINT NOT NULL,"
        "updated_at BIGINT NOT NULL,"
        "last_login BIGINT DEFAULT 0"
        ")";
    
    if (!db->executeCommand(create_users_table)) {
        std::cerr << "创建用户表失败" << std::endl;
    } else {
        std::cout << "用户表已就绪" << std::endl;
    }
    
    // 启动HTTP服务器
    if (!server->start()) {
        std::cerr << "HTTP服务器启动失败" << std::endl;
        delete server;
        db->close();
        return 1;
    }
    
    std::cout << "HTTP服务器已启动，监听端口: " << server->port() << std::endl;
    std::cout << "后端服务器正在运行..." << std::endl;
    std::cout << "按Ctrl+C终止服务器..." << std::endl;
    
    // 主线程等待，让服务器线程继续运行
    while(true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    // 正常情况下不会执行到这里，因为服务器会在接收到信号时退出
    // 但作为安全措施，我们也在这里清理资源
    if (server) {
        server->stop();
        delete server;
    }
    
    // 关闭数据库连接
    db->close();
    
    return 0;
} 