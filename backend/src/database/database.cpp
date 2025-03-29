#include "../../include/database/database.h"
#include <iostream>

Database* Database::instance = nullptr;

Database::Database() : conn(nullptr) {
}

Database* Database::getInstance() {
    if (instance == nullptr) {
        instance = new Database();
    }
    return instance;
}

bool Database::initialize(const std::string& host, const std::string& user, 
                         const std::string& password, const std::string& database, 
                         unsigned int port) {
    // 初始化MySQL连接
    conn = mysql_init(nullptr);
    if (conn == nullptr) {
        std::cerr << "MySQL初始化失败" << std::endl;
        return false;
    }
    
    // 连接到MySQL服务器
    if (mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(), 
                          database.c_str(), port, nullptr, 0) == nullptr) {
        std::cerr << "MySQL连接失败: " << mysql_error(conn) << std::endl;
        mysql_close(conn);
        conn = nullptr;
        return false;
    }
    
    // 设置字符集为UTF8
    mysql_set_character_set(conn, "utf8mb4");
    
    std::cout << "MySQL连接成功" << std::endl;
    return true;
}

MYSQL_RES* Database::executeQuery(const std::string& query) {
    if (conn == nullptr) {
        std::cerr << "MySQL连接未初始化" << std::endl;
        return nullptr;
    }
    
    if (mysql_query(conn, query.c_str()) != 0) {
        std::cerr << "查询执行失败: " << mysql_error(conn) << std::endl;
        return nullptr;
    }
    
    return mysql_store_result(conn);
}

bool Database::executeCommand(const std::string& command) {
    if (conn == nullptr) {
        std::cerr << "MySQL连接未初始化" << std::endl;
        return false;
    }
    
    if (mysql_query(conn, command.c_str()) != 0) {
        std::cerr << "命令执行失败: " << mysql_error(conn) << std::endl;
        return false;
    }
    
    return true;
}

unsigned long long Database::getAffectedRows() {
    if (conn == nullptr) {
        return 0;
    }
    
    return mysql_affected_rows(conn);
}

unsigned long long Database::getLastInsertId() {
    if (conn == nullptr) {
        return 0;
    }
    
    return mysql_insert_id(conn);
}

void Database::close() {
    if (conn != nullptr) {
        mysql_close(conn);
        conn = nullptr;
    }
}

Database::~Database() {
    close();
} 