#include "../../include/database/database.h"
#include <iostream>
#include <cstring>
#include <mutex>

Database* Database::instance = nullptr;
std::mutex Database::instanceMutex;

Database::Database() : port(3306), initialized(false) {
}

Database* Database::getInstance() {
    std::lock_guard<std::mutex> lock(instanceMutex);
    if (instance == nullptr) {
        instance = new Database();
    }
    return instance;
}

bool Database::initialize(const std::string& host, const std::string& user, 
                         const std::string& password, const std::string& database, 
                         unsigned int port) {
    // 保存连接参数
    this->host = host;
    this->user = user;
    this->password = password;
    this->dbname = database;
    this->port = port;
    
    // 初始化连接池，默认5个最小连接，20个最大连接
    bool result = DatabasePool::getInstance()->initialize(host, user, password, database, port, 5, 20);
    if (result) {
        initialized = true;
        std::cout << "数据库连接池初始化成功" << std::endl;
    } else {
        std::cerr << "数据库连接池初始化失败" << std::endl;
    }
    
    return result;
}

bool Database::isConnected() {
    return initialized;
}

MYSQL_RES* Database::executeQuery(const std::string& query) {
    std::cout << "执行查询: " << query << std::endl;
    
    if (!initialized) {
        std::cerr << "数据库连接池未初始化" << std::endl;
        return nullptr;
    }
    
    // 获取连接
    auto pool = DatabasePool::getInstance();
    auto conn = pool->getConnection();
    
    if (!conn) {
        std::cerr << "无法获取数据库连接" << std::endl;
        return nullptr;
    }
    
    // 执行查询
    MYSQL_RES* result = conn->executeQuery(query);
    
    // 释放连接回池
    pool->releaseConnection(conn);
    
    if (result) {
        std::cout << "查询执行成功，返回结果集" << std::endl;
    } else {
        std::cerr << "查询执行失败或无结果" << std::endl;
    }
    
    return result;
}

bool Database::executeCommand(const std::string& command) {
    if (!initialized) {
        std::cerr << "数据库连接池未初始化" << std::endl;
        return false;
    }
    
    // 获取连接
    auto pool = DatabasePool::getInstance();
    auto conn = pool->getConnection();
    
    if (!conn) {
        std::cerr << "无法获取数据库连接" << std::endl;
        return false;
    }
    
    // 执行命令
    bool result = conn->executeCommand(command);
    
    // 释放连接回池
    pool->releaseConnection(conn);
    
    if (result) {
        std::cout << "命令执行成功" << std::endl;
    } else {
        std::cerr << "命令执行失败" << std::endl;
    }
    
    return result;
}

unsigned long long Database::getAffectedRows() {
    if (!initialized) {
        return 0;
    }
    
    // 获取连接
    auto pool = DatabasePool::getInstance();
    auto conn = pool->getConnection();
    
    if (!conn) {
        return 0;
    }
    
    // 获取影响的行数
    unsigned long long rows = conn->getAffectedRows();
    
    // 释放连接回池
    pool->releaseConnection(conn);
    
    return rows;
}

unsigned long long Database::getLastInsertId() {
    if (!initialized) {
        return 0;
    }
    
    // 获取连接
    auto pool = DatabasePool::getInstance();
    auto conn = pool->getConnection();
    
    if (!conn) {
        return 0;
    }
    
    // 获取最后插入的ID
    unsigned long long id = conn->getLastInsertId();
    
    // 释放连接回池
    pool->releaseConnection(conn);
    
    return id;
}

std::string Database::escapeString(const std::string& str) {
    if (!initialized) {
        return str;
    }
    
    // 获取连接
    auto pool = DatabasePool::getInstance();
    auto conn = pool->getConnection();
    
    if (!conn) {
        return str;
    }
    
    // 转义字符串
    std::string escaped = conn->escapeString(str);
    
    // 释放连接回池
    pool->releaseConnection(conn);
    
    return escaped;
}

MYSQL* Database::getConnection() const {
    std::cerr << "警告: 直接获取MySQL连接对象不再支持。请使用连接池获取连接。" << std::endl;
    return nullptr;
}

void Database::maintenance() {
    if (initialized) {
        DatabasePool::getInstance()->maintenance();
    }
}

std::pair<size_t, size_t> Database::getPoolStatus() {
    if (!initialized) {
        return {0, 0};
    }
    return DatabasePool::getInstance()->getStatus();
}

void Database::close() {
    if (initialized) {
        DatabasePool::getInstance()->closeAll();
        initialized = false;
    }
}

Database::~Database() {
    close();
} 