#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <memory>
#include <mutex>
#include <utility> // 添加用于std::pair
#include "database_pool.h"

class Database {
private:
    static Database* instance;
    static std::mutex instanceMutex;  // 保护getInstance的互斥锁
    
    // 存储连接信息，用于初始化连接池
    std::string host;
    std::string user;
    std::string password;
    std::string dbname;
    unsigned int port;
    bool initialized;
    
    // 私有构造函数，防止外部实例化
    Database();

public:
    // 禁止拷贝和赋值
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    // 获取单例实例
    static Database* getInstance();
    
    // 初始化数据库连接池
    bool initialize(const std::string& host, const std::string& user, 
                   const std::string& password, const std::string& database, 
                   unsigned int port = 3306);
    
    // 检查连接池是否已初始化
    bool isConnected();
    
    // 执行SQL查询
    MYSQL_RES* executeQuery(const std::string& query);
    
    // 执行SQL命令（插入，更新，删除）
    bool executeCommand(const std::string& command);
    
    // 获取上一次操作影响的行数
    unsigned long long getAffectedRows();
    
    // 获取上一次插入操作的ID
    unsigned long long getLastInsertId();
    
    // 转义SQL字符串，防止SQL注入
    std::string escapeString(const std::string& str);
    
    // 获取MySQL连接对象（为了兼容旧代码，但不推荐使用）
    MYSQL* getConnection() const;
    
    // 定期维护连接池
    void maintenance();
    
    // 获取连接池状态
    std::pair<size_t, size_t> getPoolStatus();
    
    // 关闭数据库连接池
    void close();
    
    // 析构函数
    ~Database();
};

#endif // DATABASE_H 