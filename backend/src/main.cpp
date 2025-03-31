#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <csignal>
#include "../include/database/database.h"
#include "../include/http/http_server.h"
#include "../include/utils/jwt.h"
#include "../include/controller/controller_manager.h"
#include "../include/services/submission_service.h"
#include <json/json.h>

// 全局HTTP服务器实例
http::HttpServer* server = nullptr;

// 信号处理函数，用于优雅地关闭服务器
void signal_handler(int signal) {
    std::cout << "接收到信号 " << signal << "，正在关闭服务器..." << std::endl;
    
    // 停止评测线程
    SubmissionService::stopJudgeThread();
    
    if (server) {
        server->stop();
    }
    exit(0);
}

int main(int argc, char** argv) {
    std::cout << "启动在线评测系统后端..." << std::endl;
    
    // 解析命令行参数
    uint16_t port = 8080; // 默认端口
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            port = static_cast<uint16_t>(std::stoi(argv[i + 1]));
            i++; // 跳过下一个参数
        }
    }
    
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
    
    // 启动评测线程
    SubmissionService::ensureJudgeThreadRunning();
    std::cout << "评测服务初始化完成" << std::endl;
    
    // 创建HTTP服务器，使用httplib实现，监听指定端口
    server = new http::HttpServer(port);
    
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
    
    // 创建题目表
    std::string create_problems_table = 
        "CREATE TABLE IF NOT EXISTS problems ("
        "id INT AUTO_INCREMENT PRIMARY KEY,"
        "title VARCHAR(255) NOT NULL,"
        "description TEXT NOT NULL,"
        "input_format TEXT NOT NULL,"
        "code_template TEXT,"
        "output_format TEXT NOT NULL,"
        "difficulty ENUM('简单', '中等', '困难') NOT NULL DEFAULT '中等',"
        "time_limit INT NOT NULL DEFAULT 1000," // 默认1000ms
        "memory_limit INT NOT NULL DEFAULT 65536," // 默认64MB
        "example_input TEXT,"
        "example_output TEXT,"
        "hint TEXT,"
        "created_by INT NOT NULL,"
        "created_at BIGINT NOT NULL,"
        "updated_at BIGINT NOT NULL,"
        "status TINYINT NOT NULL DEFAULT 1" // 1: 启用, 0: 禁用
        ")";

    if (!db->executeCommand(create_problems_table)) {
        std::cerr << "创建题目表失败" << std::endl;
    } else {
        std::cout << "题目表已就绪" << std::endl;
    }
    
    // 创建测试用例表
    std::string create_testcases_table = 
        "CREATE TABLE IF NOT EXISTS testcases ("
        "id INT AUTO_INCREMENT PRIMARY KEY,"
        "problem_id INT NOT NULL,"
        "input TEXT NOT NULL,"
        "expected_output TEXT NOT NULL,"
        "is_example BOOLEAN NOT NULL DEFAULT FALSE,"
        "created_at BIGINT NOT NULL"
        ")";
    
    if (!db->executeCommand(create_testcases_table)) {
        std::cerr << "创建测试用例表失败" << std::endl;
    } else {
        std::cout << "测试用例表已就绪" << std::endl;
    }
    
    // 创建提交记录表
    std::string create_submissions_table = 
        "CREATE TABLE IF NOT EXISTS submissions ("
        "id INT AUTO_INCREMENT PRIMARY KEY,"
        "user_id INT NOT NULL,"
        "problem_id INT NOT NULL,"
        "language VARCHAR(10) NOT NULL," // cpp, java, python等
        "source_code TEXT NOT NULL,"
        "result INT NOT NULL DEFAULT 0," // 0-待评测, 1-评测中, 2-通过, 3-答案错误, 4-超时, 5-内存超限, 6-运行错误, 7-编译错误, 8-系统错误
        "score INT NOT NULL DEFAULT 0,"
        "time_used INT DEFAULT 0,"
        "memory_used INT DEFAULT 0,"
        "error_message TEXT,"
        "created_at BIGINT NOT NULL,"
        "judged_at BIGINT"
        ")";
    
    if (!db->executeCommand(create_submissions_table)) {
        std::cerr << "创建提交记录表失败" << std::endl;
    } else {
        std::cout << "提交记录表已就绪" << std::endl;
    }
    
    // 创建测试点结果表
    std::string create_test_point_results_table = 
        "CREATE TABLE IF NOT EXISTS test_point_results ("
        "id INT AUTO_INCREMENT PRIMARY KEY,"
        "submission_id INT NOT NULL,"
        "test_point_id INT NOT NULL," // 对应testcases表的id
        "result INT NOT NULL DEFAULT 0," // 同上result枚举
        "time_used INT DEFAULT 0,"
        "memory_used INT DEFAULT 0,"
        "output TEXT"
        ")";
    
    if (!db->executeCommand(create_test_point_results_table)) {
        std::cerr << "创建测试点结果表失败" << std::endl;
    } else {
        std::cout << "测试点结果表已就绪" << std::endl;
    }
    
    // 创建讨论表
    std::string create_discussions_table = 
        "CREATE TABLE IF NOT EXISTS discussions ("
        "id INT AUTO_INCREMENT PRIMARY KEY,"
        "problem_id INT,"
        "user_id INT NOT NULL,"
        "title VARCHAR(255) NOT NULL,"
        "content TEXT NOT NULL,"
        "views INT DEFAULT 0,"
        "likes INT DEFAULT 0,"
        "created_at BIGINT NOT NULL,"
        "updated_at BIGINT NOT NULL"
        ")";
    
    if (!db->executeCommand(create_discussions_table)) {
        std::cerr << "创建讨论表失败" << std::endl;
    } else {
        std::cout << "讨论表已就绪" << std::endl;
    }
    
    // 创建讨论回复表
    std::string create_discussion_replies_table = 
        "CREATE TABLE IF NOT EXISTS discussion_replies ("
        "id INT AUTO_INCREMENT PRIMARY KEY,"
        "discussion_id INT NOT NULL,"
        "user_id INT NOT NULL,"
        "parent_id INT DEFAULT 0,"
        "content TEXT NOT NULL,"
        "likes INT DEFAULT 0,"
        "created_at BIGINT NOT NULL,"
        "updated_at BIGINT NOT NULL"
        ")";
    
    if (!db->executeCommand(create_discussion_replies_table)) {
        std::cerr << "创建讨论回复表失败" << std::endl;
    } else {
        std::cout << "讨论回复表已就绪" << std::endl;
    }
    
    // 修改MySQL索引创建语法，去掉IF NOT EXISTS
    std::string create_discussions_index = 
        "CREATE INDEX idx_discussions_problem_id ON discussions(problem_id)";
    if (!db->executeCommand(create_discussions_index)) {
        std::cerr << "创建讨论表索引失败，索引可能已存在" << std::endl;
    }
    
    // 添加讨论回复表索引
    std::string create_discussion_replies_index1 = 
        "CREATE INDEX idx_discussion_replies_discussion_id ON discussion_replies(discussion_id)";
    if (!db->executeCommand(create_discussion_replies_index1)) {
        std::cerr << "创建讨论回复表索引1失败，索引可能已存在" << std::endl;
    }
    
    std::string create_discussion_replies_index2 = 
        "CREATE INDEX idx_discussion_replies_parent_id ON discussion_replies(parent_id)";
    if (!db->executeCommand(create_discussion_replies_index2)) {
        std::cerr << "创建讨论回复表索引2失败，索引可能已存在" << std::endl;
    }
    
    // 添加示例管理员帐户
    if (!db->executeCommand("INSERT INTO `cplus`.`users` (`id`, `username`, `email`, `password_hash`, `salt`, `avatar`, `role`, `status`, `created_at`, `updated_at`, `last_login`) VALUES (2, 'admin', 'admin@c.cc', '75d369ed5cb43aa6cbb62c405dd582a0e8b43aa985c4cbc230515b1247365446', '81a275083037c1df028f804739fb445e', NULL, 2, 0, 1743239731, 1743392154, 1743392154)")) {
        std::cerr << "无法插入示例管理员帐户" << std::endl;
    }
    
    std::cout << "系统初始化完成，准备监听端口 " << port << std::endl;
    
    // 启动服务器
    if (!server->start()) {
        std::cerr << "HTTP服务器启动失败，端口 " << port << " 可能已被占用" << std::endl;
        // 停止评测线程
        SubmissionService::stopJudgeThread();
        delete server;
        server = nullptr;
        // 关闭数据库连接
        db->close();
        return 1;
    }
    
    std::cout << "HTTP服务器已启动，监听端口: " << port << std::endl;
    std::cout << "后端服务器正在运行..." << std::endl;
    std::cout << "按Ctrl+C终止服务器..." << std::endl;
    
    // 服务器停止后的清理工作
    std::cout << "服务器已停止，正在进行清理..." << std::endl;

    // 停止评测线程（以防信号处理函数没有被调用）
    SubmissionService::stopJudgeThread();
    
    delete server;
    server = nullptr;
    
    // 关闭数据库连接
    db->close();
    
    return 0;
}