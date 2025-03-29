#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <csignal>
#include "../include/database/database.h"
#include "../include/http/http_server.h"
#include "../include/utils/jwt.h"
#include "../include/controller/controller_manager.h"
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
    
    // 使用控制器管理器注册所有路由
    ControllerManager controllerManager;
    controllerManager.registerAllRoutes(server);
    
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