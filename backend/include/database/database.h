#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <memory>
#include <mysql/mysql.h>

class Database {
private:
    static Database* instance;
    MYSQL* conn;
    
    // 私有构造函数，防止外部实例化
    Database();

public:
    // 禁止拷贝和赋值
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    // 获取单例实例
    static Database* getInstance();
    
    // 初始化数据库连接
    bool initialize(const std::string& host, const std::string& user, 
                   const std::string& password, const std::string& database, 
                   unsigned int port = 3306);
    
    // 执行SQL查询
    MYSQL_RES* executeQuery(const std::string& query);
    
    // 执行SQL命令（插入，更新，删除）
    bool executeCommand(const std::string& command);
    
    // 获取上一次操作影响的行数
    unsigned long long getAffectedRows();
    
    // 获取上一次插入操作的ID
    unsigned long long getLastInsertId();
    
    // 关闭数据库连接
    void close();
    
    // 析构函数
    ~Database();
};

#endif // DATABASE_H 