#ifndef USER_SERVICE_H
#define USER_SERVICE_H

#include <string>
#include <vector>
#include <map>
#include "../models/user.h"
#include <json/json.h>

class UserService {
public:
    // 用户注册
    static bool registerUser(const std::string& username, const std::string& password, 
                            const std::string& email, std::string& error_message);
    
    // 用户登录
    static std::string login(const std::string& username, const std::string& password, 
                           std::string& error_message);
    
    // 获取用户信息
    static User getUserInfo(int user_id);
    
    // 更新用户信息
    static bool updateUserInfo(int user_id, const std::map<std::string, std::string>& user_data, 
                             std::string& error_message);
    
    // 更改用户密码
    static bool changePassword(int user_id, const std::string& old_password, 
                             const std::string& new_password, std::string& error_message);
    
    // 检查用户权限
    static bool checkPermission(int user_id, int required_role);
    
    // 检查用户是否存在
    static bool isUserExist(const std::string& username);
    
    // 检查邮箱是否存在
    static bool isEmailExist(const std::string& email);
    
    // 获取用户列表
    static std::vector<User> getUserList(int offset = 0, int limit = 10);
    
    // 更改用户状态（封禁/解禁）
    static bool changeUserStatus(int user_id, UserStatus status, std::string& error_message);
    
    // 获取用户排行榜
    static bool getLeaderboard(int offset, int limit, const std::string& time_range, 
                              int current_user_id, Json::Value& leaderboard_data, 
                              int& total, std::string& error_message);
    
    // 更新用户排行榜统计信息
    static bool updateUserLeaderboardStats(int user_id, int problem_id, bool is_accepted);
    
    // ===== 管理员用户管理相关服务方法 =====
    
    // 获取所有用户（支持搜索和筛选）
    static bool getAllUsers(int offset, int limit, const std::string& search_term, 
                           int role_filter, int status_filter, Json::Value& user_data, 
                           int& total, std::string& error_message);
    
    // 通过管理员创建用户
    static bool createUserByAdmin(const std::string& username, const std::string& password, 
                                const std::string& email, int role, std::string& error_message);
    
    // 通过管理员更新用户信息
    static bool updateUserByAdmin(int user_id, const Json::Value& user_data, 
                                std::string& error_message);
    
    // 删除用户（软删除）
    static bool deleteUser(int user_id, std::string& error_message);
    
    // 更改用户角色
    static bool changeUserRole(int user_id, UserRole role, std::string& error_message);
    
    // 重置用户密码
    static bool resetUserPassword(int user_id, const std::string& new_password, 
                                std::string& error_message);
    
private:
    // 验证密码强度
    static bool validatePassword(const std::string& password);
    
    // 验证用户名格式
    static bool validateUsername(const std::string& username);
    
    // 验证邮箱格式
    static bool validateEmail(const std::string& email);
};

#endif // USER_SERVICE_H 