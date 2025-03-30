#ifndef DATABASE_POOL_H
#define DATABASE_POOL_H

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <chrono>
#include <iostream>
#include <mysql/mysql.h>

// MySQL连接包装类
class MySQLConnection {
private:
    MYSQL* conn;
    bool busy;
    time_t lastUsed;

public:
    MySQLConnection() : conn(nullptr), busy(false), lastUsed(0) {}
    
    ~MySQLConnection() {
        if (conn) {
            mysql_close(conn);
            conn = nullptr;
        }
    }
    
    bool connect(const std::string& host, const std::string& user, 
                const std::string& password, const std::string& database, 
                unsigned int port) {
        // 初始化连接并连接到服务器
        conn = mysql_init(nullptr);
        if (!conn) return false;
        
        // 设置重连选项 - 使用推荐的新方法
        // 替代弃用的 MYSQL_OPT_RECONNECT
        bool reconnect = true;
        mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);
        
        // 设置连接超时
        unsigned int timeout = 10;
        mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
        
        // 设置读写超时
        unsigned int read_timeout = 30;
        mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, &read_timeout);
        
        unsigned int write_timeout = 30;
        mysql_options(conn, MYSQL_OPT_WRITE_TIMEOUT, &write_timeout);
        
        if (!mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(),
                               database.c_str(), port, nullptr, 0)) {
            std::cerr << "连接失败: " << mysql_error(conn) << std::endl;
            mysql_close(conn);
            conn = nullptr;
            return false;
        }
        
        // 设置UTF8字符集
        mysql_set_character_set(conn, "utf8mb4");
        lastUsed = time(nullptr);
        return true;
    }
    
    bool isValid() {
        if (!conn) return false;
        return mysql_ping(conn) == 0;
    }
    
    MYSQL* getConnection() {
        return conn;
    }
    
    void setBusy(bool status) {
        busy = status;
        if (status) {
            lastUsed = time(nullptr);
        }
    }
    
    bool isBusy() const {
        return busy;
    }
    
    time_t getLastUsed() const {
        return lastUsed;
    }
    
    // 执行查询并返回结果集
    MYSQL_RES* executeQuery(const std::string& query) {
        if (!conn) return nullptr;
        
        if (mysql_query(conn, query.c_str()) != 0) {
            std::cerr << "查询执行失败: " << mysql_error(conn) << std::endl;
            return nullptr;
        }
        
        return mysql_store_result(conn);
    }
    
    // 执行命令
    bool executeCommand(const std::string& command) {
        if (!conn) return false;
        
        return mysql_query(conn, command.c_str()) == 0;
    }
    
    // 转义字符串
    std::string escapeString(const std::string& str) {
        if (!conn) return str;
        
        size_t length = str.length();
        std::vector<char> buffer(length * 2 + 1);
        
        mysql_real_escape_string(conn, buffer.data(), str.c_str(), length);
        return std::string(buffer.data());
    }
    
    // 获取影响的行数
    unsigned long long getAffectedRows() {
        if (!conn) return 0;
        return mysql_affected_rows(conn);
    }
    
    // 获取最后插入的ID
    unsigned long long getLastInsertId() {
        if (!conn) return 0;
        return mysql_insert_id(conn);
    }
};

// 数据库连接池类
class DatabasePool {
private:
    static DatabasePool* instance;
    static std::mutex instanceMutex;
    
    std::vector<std::shared_ptr<MySQLConnection>> connections;
    std::mutex poolMutex;
    std::condition_variable cv;
    
    std::string host;
    std::string user;
    std::string password;
    std::string database;
    unsigned int port;
    
    size_t maxConnections;
    size_t minConnections;
    time_t timeout; // 连接最大空闲时间（秒）
    
    // 私有构造函数 - 修复初始化顺序
    DatabasePool() : 
        host(""),
        user(""),
        password(""),
        database(""),
        port(3306),
        maxConnections(10),
        minConnections(5),
        timeout(300)
    {}
    
    // 创建新连接
    std::shared_ptr<MySQLConnection> createConnection() {
        auto conn = std::make_shared<MySQLConnection>();
        if (conn->connect(host, user, password, database, port)) {
            return conn;
        }
        return nullptr;
    }
    
public:
    // 获取单例实例
    static DatabasePool* getInstance() {
        std::lock_guard<std::mutex> lock(instanceMutex);
        if (instance == nullptr) {
            instance = new DatabasePool();
        }
        return instance;
    }
    
    // 初始化连接池
    bool initialize(const std::string& host, const std::string& user,
                   const std::string& password, const std::string& database,
                   unsigned int port = 3306, size_t minConn = 5, size_t maxConn = 10) {
        std::lock_guard<std::mutex> lock(poolMutex);
        
        this->host = host;
        this->user = user;
        this->password = password;
        this->database = database;
        this->port = port;
        this->minConnections = minConn;
        this->maxConnections = maxConn;
        
        // 预创建最小数量的连接
        for (size_t i = 0; i < minConnections; i++) {
            auto conn = createConnection();
            if (conn) {
                connections.push_back(conn);
            } else {
                std::cerr << "无法创建初始数据库连接" << std::endl;
                return false;
            }
        }
        
        std::cout << "数据库连接池初始化成功，创建了 " << connections.size() << " 个连接" << std::endl;
        return !connections.empty();
    }
    
    // 获取一个可用连接
    std::shared_ptr<MySQLConnection> getConnection(int timeoutMs = 5000) {
        std::unique_lock<std::mutex> lock(poolMutex);
        
        auto startTime = std::chrono::steady_clock::now();
        auto endTime = startTime + std::chrono::milliseconds(timeoutMs);
        
        while (true) {
            // 查找空闲连接
            for (auto& conn : connections) {
                if (!conn->isBusy()) {
                    if (!conn->isValid()) {
                        // 连接已断开，尝试重新连接
                        if (conn->connect(host, user, password, database, port)) {
                            conn->setBusy(true);
                            return conn;
                        }
                        continue;
                    }
                    
                    conn->setBusy(true);
                    return conn;
                }
            }
            
            // 如果没有空闲连接但连接数小于最大值，创建新连接
            if (connections.size() < maxConnections) {
                auto conn = createConnection();
                if (conn) {
                    conn->setBusy(true);
                    connections.push_back(conn);
                    return conn;
                }
            }
            
            // 等待一个连接变为可用或超时
            if (cv.wait_until(lock, endTime) == std::cv_status::timeout) {
                std::cerr << "获取数据库连接超时" << std::endl;
                return nullptr;
            }
        }
    }
    
    // 释放连接回池
    void releaseConnection(std::shared_ptr<MySQLConnection> conn) {
        if (!conn) return;
        
        {
            std::lock_guard<std::mutex> lock(poolMutex);
            conn->setBusy(false);
        }
        
        // 通知一个正在等待连接的线程
        cv.notify_one();
    }
    
    // 连接池维护（清理空闲连接）
    void maintenance() {
        std::lock_guard<std::mutex> lock(poolMutex);
        
        time_t now = time(nullptr);
        auto it = connections.begin();
        
        while (it != connections.end()) {
            auto conn = *it;
            
            // 如果连接空闲且超时，且连接数大于最小连接数，则移除
            if (!conn->isBusy() && 
                (now - conn->getLastUsed() > timeout) && 
                connections.size() > minConnections) {
                it = connections.erase(it);
                std::cout << "关闭空闲连接，当前连接数: " << connections.size() << std::endl;
            } else {
                ++it;
            }
        }
    }
    
    // 关闭所有连接
    void closeAll() {
        std::lock_guard<std::mutex> lock(poolMutex);
        connections.clear();
    }
    
    // 获取连接池状态
    std::pair<size_t, size_t> getStatus() {
        std::lock_guard<std::mutex> lock(poolMutex);
        size_t busyCount = 0;
        for (const auto& conn : connections) {
            if (conn->isBusy()) {
                busyCount++;
            }
        }
        return {connections.size(), busyCount};
    }
    
    ~DatabasePool() {
        closeAll();
    }
};

#endif // DATABASE_POOL_H 