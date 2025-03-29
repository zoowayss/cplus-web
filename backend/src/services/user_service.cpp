#include "../../include/services/user_service.h"
#include "../../include/database/database.h"
#include "../../include/utils/jwt.h"
#include <regex>
#include <iostream>
#include <ctime>
#include <map>
#include <mysql/mysql.h>

// 验证密码强度
bool UserService::validatePassword(const std::string& password) {
    // 密码长度至少6位
    if (password.length() < 6) {
        return false;
    }
    
    // 密码复杂度检查（例如必须包含字母和数字）
    bool has_letter = false;
    bool has_digit = false;
    
    for (char c : password) {
        if (isalpha(c)) {
            has_letter = true;
        } else if (isdigit(c)) {
            has_digit = true;
        }
        
        if (has_letter && has_digit) {
            break;
        }
    }
    
    return has_letter && has_digit;
}

// 验证用户名格式
bool UserService::validateUsername(const std::string& username) {
    // 用户名长度在3-20之间
    if (username.length() < 3 || username.length() > 20) {
        return false;
    }
    
    // 用户名只能包含字母、数字和下划线
    std::regex pattern("^[a-zA-Z0-9_]+$");
    return std::regex_match(username, pattern);
}

// 验证邮箱格式
bool UserService::validateEmail(const std::string& email) {
    // 简单的邮箱格式验证
    std::regex pattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return std::regex_match(email, pattern);
}

// 检查用户是否存在
bool UserService::isUserExist(const std::string& username) {
    Database* db = Database::getInstance();
    
    std::string query = "SELECT COUNT(*) FROM users WHERE username = '" + username + "'";
    MYSQL_RES* result = db->executeQuery(query);
    
    if (!result) {
        return false;
    }
    
    MYSQL_ROW row = mysql_fetch_row(result);
    bool exists = row && std::stoi(row[0]) > 0;
    
    mysql_free_result(result);
    return exists;
}

// 检查邮箱是否存在
bool UserService::isEmailExist(const std::string& email) {
    Database* db = Database::getInstance();
    
    std::string query = "SELECT COUNT(*) FROM users WHERE email = '" + email + "'";
    MYSQL_RES* result = db->executeQuery(query);
    
    if (!result) {
        return false;
    }
    
    MYSQL_ROW row = mysql_fetch_row(result);
    bool exists = row && std::stoi(row[0]) > 0;
    
    mysql_free_result(result);
    return exists;
}

// 用户注册
bool UserService::registerUser(const std::string& username, const std::string& password, 
                              const std::string& email, std::string& error_message) {
    // 验证输入
    if (!validateUsername(username)) {
        error_message = "用户名格式不正确，只能包含字母、数字和下划线，长度3-20位";
        return false;
    }
    
    if (!validatePassword(password)) {
        error_message = "密码强度不够，密码必须至少6位，且包含字母和数字";
        return false;
    }
    
    if (!validateEmail(email)) {
        error_message = "邮箱格式不正确";
        return false;
    }
    
    // 检查用户名和邮箱是否已存在
    if (isUserExist(username)) {
        error_message = "用户名已存在";
        return false;
    }
    
    if (isEmailExist(email)) {
        error_message = "邮箱已注册";
        return false;
    }
    
    // 生成密码哈希和盐
    auto [password_hash, salt] = User::generatePasswordHash(password);
    
    // 获取数据库连接
    Database* db = Database::getInstance();
    
    // 准备插入语句
    std::string query = "INSERT INTO users (username, email, password_hash, salt, role, status, created_at, updated_at) VALUES (";
    query += "'" + username + "', ";
    query += "'" + email + "', ";
    query += "'" + password_hash + "', ";
    query += "'" + salt + "', ";
    query += std::to_string(static_cast<int>(UserRole::STUDENT)) + ", ";
    query += std::to_string(static_cast<int>(UserStatus::NORMAL)) + ", ";
    
    time_t now = time(nullptr);
    query += std::to_string(now) + ", ";
    query += std::to_string(now) + ")";
    
    // 执行插入
    bool success = db->executeCommand(query);
    
    if (!success) {
        error_message = "注册失败，数据库错误";
        return false;
    }
    
    return true;
}

// 用户登录
std::string UserService::login(const std::string& username, const std::string& password, 
                             std::string& error_message) {
    // 获取数据库连接
    Database* db = Database::getInstance();
    
    // 查询用户
    std::string query = "SELECT * FROM users WHERE username = '" + username + "'";
    MYSQL_RES* result = db->executeQuery(query);
    
    if (!result) {
        error_message = "登录失败，数据库错误";
        return "";
    }
    
    if (mysql_num_rows(result) == 0) {
        mysql_free_result(result);
        error_message = "用户名或密码错误";
        return "";
    }
    
    // 获取用户数据
    User user = User::fromDatabaseResult(result, 0);
    mysql_free_result(result);
    
    // 检查用户状态
    if (user.getStatus() != UserStatus::NORMAL) {
        error_message = "账号已禁用或删除";
        return "";
    }
    
    // 验证密码
    if (!user.checkPassword(password)) {
        error_message = "用户名或密码错误";
        return "";
    }
    
    // 更新最后登录时间
    user.updateLastLogin();
    std::string update_query = "UPDATE users SET last_login = " + std::to_string(user.getLastLogin()) + 
                              ", updated_at = " + std::to_string(user.getUpdatedAt()) + 
                              " WHERE id = " + std::to_string(user.getId());
    db->executeCommand(update_query);
    
    // 生成JWT令牌
    std::map<std::string, std::string> payload;
    payload["user_id"] = std::to_string(user.getId());
    payload["username"] = user.getUsername();
    payload["role"] = std::to_string(static_cast<int>(user.getRole()));
    
    // 令牌有效期24小时
    return JWT::generateToken(payload, 86400);
}

// 获取用户信息
User UserService::getUserInfo(int user_id) {
    Database* db = Database::getInstance();
    
    std::string query = "SELECT * FROM users WHERE id = " + std::to_string(user_id);
    MYSQL_RES* result = db->executeQuery(query);
    
    if (!result || mysql_num_rows(result) == 0) {
        if (result) mysql_free_result(result);
        return User(); // 返回空用户对象
    }
    
    User user = User::fromDatabaseResult(result, 0);
    mysql_free_result(result);
    
    return user;
}

// 更新用户信息
bool UserService::updateUserInfo(int user_id, const std::map<std::string, std::string>& user_data, 
                               std::string& error_message) {
    // 获取数据库连接
    Database* db = Database::getInstance();
    
    // 检查用户是否存在
    User user = getUserInfo(user_id);
    if (user.getId() == 0) {
        error_message = "用户不存在";
        return false;
    }
    
    // 构建更新语句
    std::string query = "UPDATE users SET ";
    bool first = true;
    
    for (const auto& [key, value] : user_data) {
        // 只允许更新某些字段
        if (key != "username" && key != "email" && key != "avatar") {
            continue;
        }
        
        // 如果要更改用户名，需要检查是否已存在
        if (key == "username" && value != user.getUsername()) {
            if (!validateUsername(value)) {
                error_message = "用户名格式不正确";
                return false;
            }
            
            if (isUserExist(value)) {
                error_message = "用户名已存在";
                return false;
            }
        }
        
        // 如果要更改邮箱，需要检查是否已存在
        if (key == "email" && value != user.getEmail()) {
            if (!validateEmail(value)) {
                error_message = "邮箱格式不正确";
                return false;
            }
            
            if (isEmailExist(value)) {
                error_message = "邮箱已注册";
                return false;
            }
        }
        
        if (!first) {
            query += ", ";
        }
        
        query += key + " = '" + value + "'";
        first = false;
    }
    
    if (first) {
        // 没有需要更新的字段
        return true;
    }
    
    // 更新时间
    time_t now = time(nullptr);
    query += ", updated_at = " + std::to_string(now);
    
    // 添加条件
    query += " WHERE id = " + std::to_string(user_id);
    
    // 执行更新
    bool success = db->executeCommand(query);
    
    if (!success) {
        error_message = "更新失败，数据库错误";
        return false;
    }
    
    return true;
}

// 更改用户密码
bool UserService::changePassword(int user_id, const std::string& old_password, 
                               const std::string& new_password, std::string& error_message) {
    // 验证新密码强度
    if (!validatePassword(new_password)) {
        error_message = "新密码强度不够，密码必须至少6位，且包含字母和数字";
        return false;
    }
    
    // 获取用户信息
    User user = getUserInfo(user_id);
    if (user.getId() == 0) {
        error_message = "用户不存在";
        return false;
    }
    
    // 验证旧密码
    if (!user.checkPassword(old_password)) {
        error_message = "旧密码不正确";
        return false;
    }
    
    // 生成新的密码哈希和盐
    auto [password_hash, salt] = User::generatePasswordHash(new_password);
    
    // 获取数据库连接
    Database* db = Database::getInstance();
    
    // 构建更新语句
    time_t now = time(nullptr);
    std::string query = "UPDATE users SET ";
    query += "password_hash = '" + password_hash + "', ";
    query += "salt = '" + salt + "', ";
    query += "updated_at = " + std::to_string(now);
    query += " WHERE id = " + std::to_string(user_id);
    
    // 执行更新
    bool success = db->executeCommand(query);
    
    if (!success) {
        error_message = "密码更新失败，数据库错误";
        return false;
    }
    
    return true;
}

// 检查用户权限
bool UserService::checkPermission(int user_id, int required_role) {
    User user = getUserInfo(user_id);
    
    if (user.getId() == 0) {
        return false;
    }
    
    // 检查用户状态
    if (user.getStatus() != UserStatus::NORMAL) {
        return false;
    }
    
    // 检查用户角色
    return static_cast<int>(user.getRole()) >= required_role;
}

// 获取用户列表
std::vector<User> UserService::getUserList(int offset, int limit) {
    std::vector<User> users;
    
    // 获取数据库连接
    Database* db = Database::getInstance();
    
    // 构建查询语句
    std::string query = "SELECT * FROM users ORDER BY id LIMIT " + 
                       std::to_string(limit) + " OFFSET " + std::to_string(offset);
    
    // 执行查询
    MYSQL_RES* result = db->executeQuery(query);
    
    if (!result) {
        return users;
    }
    
    // 获取结果行数
    my_ulonglong num_rows = mysql_num_rows(result);
    
    // 提取用户数据
    for (my_ulonglong i = 0; i < num_rows; i++) {
        User user = User::fromDatabaseResult(result, i);
        users.push_back(user);
    }
    
    mysql_free_result(result);
    
    return users;
}

// 更改用户状态
bool UserService::changeUserStatus(int user_id, UserStatus status, std::string& error_message) {
    // 检查用户是否存在
    User user = getUserInfo(user_id);
    if (user.getId() == 0) {
        error_message = "用户不存在";
        return false;
    }
    
    // 不能更改自己的状态
    // 注意：在实际应用中，这里应该检查当前登录用户的ID，而不是直接判断
    if (user.getId() == user_id) {
        error_message = "不能更改自己的状态";
        return false;
    }
    
    // 获取数据库连接
    Database* db = Database::getInstance();
    
    // 构建更新语句
    time_t now = time(nullptr);
    std::string query = "UPDATE users SET ";
    query += "status = " + std::to_string(static_cast<int>(status)) + ", ";
    query += "updated_at = " + std::to_string(now);
    query += " WHERE id = " + std::to_string(user_id);
    
    // 执行更新
    bool success = db->executeCommand(query);
    
    if (!success) {
        error_message = "状态更新失败，数据库错误";
        return false;
    }
    
    return true;
} 