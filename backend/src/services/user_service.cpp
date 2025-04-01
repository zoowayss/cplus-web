#include "../../include/services/user_service.h"
#include "../../include/database/database.h"
#include "../../include/utils/jwt.h"
#include <regex>
#include <iostream>
#include <ctime>
#include <map>
#include <mysql/mysql.h>
#include <json/json.h>

// 验证密码强度
bool UserService::validatePassword(const std::string &password)
{
    // 密码长度至少6位
    return password.length() >= 6;
}

// 验证用户名格式
bool UserService::validateUsername(const std::string &username)
{
    // 用户名长度在3-20之间，只能包含字母、数字和下划线
    std::regex pattern("^[a-zA-Z0-9_]{3,20}$");
    return std::regex_match(username, pattern);
}

// 验证邮箱格式
bool UserService::validateEmail(const std::string &email)
{
    // 简单的邮箱格式验证
    std::regex pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return std::regex_match(email, pattern);
}

// 检查用户是否存在
bool UserService::isUserExist(const std::string &username)
{
    Database *db = Database::getInstance();

    std::string query = "SELECT COUNT(*) FROM users WHERE username = '" + username + "'";
    MYSQL_RES *result = db->executeQuery(query);

    if (!result)
    {
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    bool exists = (row && row[0] && std::stoi(row[0]) > 0);

    mysql_free_result(result);
    return exists;
}

// 检查邮箱是否存在
bool UserService::isEmailExist(const std::string &email)
{
    Database *db = Database::getInstance();

    std::string query = "SELECT COUNT(*) FROM users WHERE email = '" + email + "'";
    MYSQL_RES *result = db->executeQuery(query);

    if (!result)
    {
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    bool exists = (row && row[0] && std::stoi(row[0]) > 0);

    mysql_free_result(result);
    return exists;
}

// 用户注册
bool UserService::registerUser(const std::string &username, const std::string &email,
                               const std::string &password, std::string &error_message)
{
    // 验证用户名
    if (!validateUsername(username))
    {
        error_message = "用户名格式不正确";
        return false;
    }

    // 验证邮箱
    if (!validateEmail(email))
    {
        error_message = "邮箱格式不正确";
        return false;
    }

    // 验证密码
    if (!validatePassword(password))
    {
        error_message = "密码长度不能少于6位";
        return false;
    }

    // 检查用户名是否已存在
    if (isUserExist(username))
    {
        error_message = "用户名已存在";
        return false;
    }

    // 检查邮箱是否已存在
    if (isEmailExist(email))
    {
        error_message = "邮箱已注册";
        return false;
    }

    // 生成密码哈希和盐 - C++11兼容写法
    std::pair<std::string, std::string> password_data = User::generatePasswordHash(password);
    std::string password_hash = password_data.first;
    std::string salt = password_data.second;

    // 获取数据库连接
    Database *db = Database::getInstance();

    // 准备插入语句，添加排行榜相关字段
    std::string query = "INSERT INTO users (username, email, password_hash, salt, role, status, created_at, updated_at, "
                        "solved_count, submission_count, score, easy_count, medium_count, hard_count) VALUES (";
    query += "'" + username + "', ";
    query += "'" + email + "', ";
    query += "'" + password_hash + "', ";
    query += "'" + salt + "', ";
    query += std::to_string(static_cast<int>(UserRole::STUDENT)) + ", ";
    query += std::to_string(static_cast<int>(UserStatus::NORMAL)) + ", ";

    time_t now = time(nullptr);
    query += std::to_string(now) + ", ";
    query += std::to_string(now) + ", ";

    // 添加排行榜相关字段的初始值
    query += "0, "; // solved_count
    query += "0, "; // submission_count
    query += "0, "; // score
    query += "0, "; // easy_count
    query += "0, "; // medium_count
    query += "0)";  // hard_count

    // 执行插入
    bool success = db->executeCommand(query);

    if (!success)
    {
        error_message = "注册失败，数据库错误";
        return false;
    }

    return true;
}

// 用户登录
std::string UserService::login(const std::string &username, const std::string &password,
                               std::string &error_message)
{
    // 获取数据库连接
    Database *db = Database::getInstance();

    // 查询用户
    std::string query = "SELECT * FROM users WHERE username = '" + username + "'";
    MYSQL_RES *result = db->executeQuery(query);

    if (!result)
    {
        error_message = "登录失败，数据库错误";
        return "";
    }

    if (mysql_num_rows(result) == 0)
    {
        mysql_free_result(result);
        error_message = "用户名或密码错误";
        return "";
    }

    // 获取用户数据
    User user = User::fromDatabaseResult(result, 0);
    mysql_free_result(result);

    // 检查用户状态
    if (user.getStatus() != UserStatus::NORMAL)
    {
        error_message = "账号已禁用或删除";
        return "";
    }

    // 验证密码
    if (!user.checkPassword(password))
    {
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
User UserService::getUserInfo(int user_id)
{
    Database *db = Database::getInstance();

    std::string query = "SELECT * FROM users WHERE id = " + std::to_string(user_id);
    MYSQL_RES *result = db->executeQuery(query);

    if (!result || mysql_num_rows(result) == 0)
    {
        if (result)
            mysql_free_result(result);
        return User(); // 返回空用户对象
    }

    User user = User::fromDatabaseResult(result, 0);
    mysql_free_result(result);

    return user;
}

// 更新用户信息
bool UserService::updateUserInfo(int user_id, const std::map<std::string, std::string> &user_data,
                                 std::string &error_message)
{
    // 获取数据库连接
    Database *db = Database::getInstance();

    // 检查用户是否存在
    User user = getUserInfo(user_id);
    if (user.getId() == 0)
    {
        error_message = "用户不存在";
        return false;
    }

    // 构建更新语句
    std::string query = "UPDATE users SET ";
    bool first = true;

    // C++11兼容的迭代方式
    for (const auto &pair : user_data)
    {
        const std::string &key = pair.first;
        const std::string &value = pair.second;

        // 只允许更新某些字段
        if (key != "username" && key != "email" && key != "avatar")
        {
            continue;
        }

        // 如果要更改用户名，需要检查是否已存在
        if (key == "username" && value != user.getUsername())
        {
            if (!validateUsername(value))
            {
                error_message = "用户名格式不正确";
                return false;
            }

            if (isUserExist(value))
            {
                error_message = "用户名已存在";
                return false;
            }
        }

        // 如果要更改邮箱，需要检查是否已存在
        if (key == "email" && value != user.getEmail())
        {
            if (!validateEmail(value))
            {
                error_message = "邮箱格式不正确";
                return false;
            }

            if (isEmailExist(value))
            {
                error_message = "邮箱已注册";
                return false;
            }
        }

        if (!first)
        {
            query += ", ";
        }

        query += key + " = '" + value + "'";
        first = false;
    }

    if (first)
    {
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

    if (!success)
    {
        error_message = "更新失败，数据库错误";
        return false;
    }

    return true;
}

// 更改用户密码
bool UserService::changePassword(int user_id, const std::string &old_password,
                                 const std::string &new_password, std::string &error_message)
{
    // 验证新密码强度
    if (!validatePassword(new_password))
    {
        error_message = "新密码长度不能少于6位";
        return false;
    }

    // 获取用户信息
    User user = getUserInfo(user_id);
    if (user.getId() == 0)
    {
        error_message = "用户不存在";
        return false;
    }

    // 验证旧密码
    if (!user.checkPassword(old_password))
    {
        error_message = "旧密码不正确";
        return false;
    }

    // 生成新的密码哈希和盐 - C++11兼容写法
    std::pair<std::string, std::string> password_data = User::generatePasswordHash(new_password);
    std::string password_hash = password_data.first;
    std::string salt = password_data.second;

    // 获取数据库连接
    Database *db = Database::getInstance();

    // 构建更新语句
    time_t now = time(nullptr);
    std::string query = "UPDATE users SET ";
    query += "password_hash = '" + password_hash + "', ";
    query += "salt = '" + salt + "', ";
    query += "updated_at = " + std::to_string(now);
    query += " WHERE id = " + std::to_string(user_id);

    // 执行更新
    bool success = db->executeCommand(query);

    if (!success)
    {
        error_message = "密码更新失败，数据库错误";
        return false;
    }

    return true;
}

// 检查用户权限
bool UserService::checkPermission(int user_id, int required_role)
{
    User user = getUserInfo(user_id);

    if (user.getId() == 0)
    {
        return false;
    }

    // 检查用户状态
    if (user.getStatus() != UserStatus::NORMAL)
    {
        return false;
    }

    // 检查用户角色
    return static_cast<int>(user.getRole()) >= required_role;
}

// 获取用户列表
std::vector<User> UserService::getUserList(int offset, int limit)
{
    std::vector<User> users;

    // 获取数据库连接
    Database *db = Database::getInstance();

    // 构建查询语句
    std::string query = "SELECT * FROM users ORDER BY id LIMIT " +
                        std::to_string(limit) + " OFFSET " + std::to_string(offset);

    // 执行查询
    MYSQL_RES *result = db->executeQuery(query);

    if (!result)
    {
        return users;
    }

    // 获取结果行数
    my_ulonglong num_rows = mysql_num_rows(result);

    // 提取用户数据
    for (my_ulonglong i = 0; i < num_rows; i++)
    {
        User user = User::fromDatabaseResult(result, i);
        users.push_back(user);
    }

    mysql_free_result(result);

    return users;
}

// 更改用户状态
bool UserService::changeUserStatus(int user_id, UserStatus status, std::string &error_message)
{
    // 检查用户是否存在
    User user = getUserInfo(user_id);
    if (user.getId() == 0)
    {
        error_message = "用户不存在";
        return false;
    }

    // 不能更改自己的状态
    // 注意：在实际应用中，这里应该检查当前登录用户的ID，而不是直接判断
    if (user.getId() == user_id)
    {
        error_message = "不能更改自己的状态";
        return false;
    }

    // 获取数据库连接
    Database *db = Database::getInstance();

    // 构建更新语句
    time_t now = time(nullptr);
    std::string query = "UPDATE users SET ";
    query += "status = " + std::to_string(static_cast<int>(status)) + ", ";
    query += "updated_at = " + std::to_string(now);
    query += " WHERE id = " + std::to_string(user_id);

    // 执行更新
    bool success = db->executeCommand(query);

    if (!success)
    {
        error_message = "状态更新失败，数据库错误";
        return false;
    }

    return true;
}

// 获取用户排行榜
bool UserService::getLeaderboard(int offset, int limit, const std::string &time_range,
                                 int current_user_id, Json::Value &leaderboard_data,
                                 int &total, std::string &error_message)
{
    // 获取数据库连接
    Database *db = Database::getInstance();

    // 先查询数据库确认表结构
    std::string table_check_query = "SHOW TABLES LIKE 'submissions'";
    MYSQL_RES *table_check_result = db->executeQuery(table_check_query);
    bool has_submissions_table = false;

    if (table_check_result && mysql_num_rows(table_check_result) > 0)
    {
        has_submissions_table = true;
        mysql_free_result(table_check_result);
    }
    else if (table_check_result)
    {
        mysql_free_result(table_check_result);
    }

    // 根据数据库结构选择不同的查询
    std::string base_query;

    if (has_submissions_table)
    {
        // 使用submissions表进行连接查询，获取真实的统计数据
        base_query = "SELECT u.id, u.username, IFNULL(u.avatar, '') as avatar, "
                    // 计算已解决的题目数量（distinct problem_id且result=2）
                    "COUNT(DISTINCT CASE WHEN s.result = 2 THEN s.problem_id ELSE NULL END) as solved_count, "
                    // 计算总提交数
                    "COUNT(s.id) as submission_count, "
                    // 计算分数：简单题10分，中等题20分，困难题30分
                    "SUM(CASE WHEN s.result = 2 THEN "
                    "   CASE WHEN p.difficulty = '简单' THEN 10 "
                    "   WHEN p.difficulty = '中等' THEN 20 "
                    "   WHEN p.difficulty = '困难' THEN 30 "
                    "   ELSE 0 END "
                    "ELSE 0 END) as score, "
                    // 计算各难度已解决的题目数量
                    "COUNT(DISTINCT CASE WHEN s.result = 2 AND p.difficulty = '简单' THEN s.problem_id ELSE NULL END) as easy_count, "
                    "COUNT(DISTINCT CASE WHEN s.result = 2 AND p.difficulty = '中等' THEN s.problem_id ELSE NULL END) as medium_count, "
                    "COUNT(DISTINCT CASE WHEN s.result = 2 AND p.difficulty = '困难' THEN s.problem_id ELSE NULL END) as hard_count, "
                    // 计算通过率
                    "CASE WHEN COUNT(s.id) > 0 THEN "
                    "ROUND(COUNT(DISTINCT CASE WHEN s.result = 2 THEN s.problem_id ELSE NULL END) * 100.0 / "
                    "COUNT(DISTINCT s.problem_id), 2) "
                    "ELSE 0 END as acceptance_rate "
                    "FROM users u "
                    "LEFT JOIN submissions s ON u.id = s.user_id "
                    "LEFT JOIN problems p ON s.problem_id = p.id "
                    "WHERE u.role < 2 "; // 排除管理员
    }
    else
    {
        // 没有submissions表，只查询users表
        base_query = "SELECT u.id, u.username, IFNULL(u.avatar, '') as avatar, "
                    "u.solved_count, "
                    "u.submission_count, "
                    "u.score, "
                    "u.easy_count, "
                    "u.medium_count, "
                    "u.hard_count, "
                    "CASE WHEN u.submission_count > 0 THEN "
                    "ROUND(u.solved_count * 100.0 / u.submission_count, 2) "
                    "ELSE 0 END as acceptance_rate "
                    "FROM users u "
                    "WHERE u.role < 2 "; // 排除管理员
    }

    // 根据时间范围添加条件
    std::string time_condition = "";
    if (time_range == "day")
    {
        if (has_submissions_table)
        {
            time_condition = " AND DATE(FROM_UNIXTIME(s.created_at)) = CURDATE() ";
        }
        else
        {
            time_condition = " AND DATE(FROM_UNIXTIME(u.last_login)) = CURDATE() ";
        }
    }
    else if (time_range == "week")
    {
        if (has_submissions_table)
        {
            time_condition = " AND YEARWEEK(FROM_UNIXTIME(s.created_at)) = YEARWEEK(NOW()) ";
        }
        else
        {
            time_condition = " AND YEARWEEK(FROM_UNIXTIME(u.last_login)) = YEARWEEK(NOW()) ";
        }
    }
    else if (time_range == "month")
    {
        if (has_submissions_table)
        {
            time_condition = " AND MONTH(FROM_UNIXTIME(s.created_at)) = MONTH(NOW()) AND YEAR(FROM_UNIXTIME(s.created_at)) = YEAR(NOW()) ";
        }
        else
        {
            time_condition = " AND MONTH(FROM_UNIXTIME(u.last_login)) = MONTH(NOW()) AND YEAR(FROM_UNIXTIME(u.last_login)) = YEAR(NOW()) ";
        }
    }

    // 如果使用submissions表，需要添加GROUP BY
    std::string group_by = "";
    if (has_submissions_table)
    {
        group_by = " GROUP BY u.id, u.username, u.avatar ";
    }

    // 添加排序规则
    std::string order_query = "ORDER BY solved_count DESC, acceptance_rate DESC, submission_count ASC ";

    // 添加分页
    std::string limit_query = "LIMIT " + std::to_string(limit) + " OFFSET " + std::to_string(offset);

    // 计算总数查询（计算符合条件的用户数量）
    std::string count_query;
    if (has_submissions_table && time_range != "all")
    {
        count_query = "SELECT COUNT(DISTINCT u.id) FROM users u "
                     "LEFT JOIN submissions s ON u.id = s.user_id "
                     "WHERE u.role < 2 " + time_condition;
    }
    else
    {
        count_query = "SELECT COUNT(*) FROM users u WHERE u.role < 2 " + time_condition;
    }

    // 执行计算总数查询
    MYSQL_RES *count_result = db->executeQuery(count_query);

    if (!count_result)
    {
        error_message = "获取排行榜数据失败，数据库错误";
        return false;
    }

    MYSQL_ROW count_row = mysql_fetch_row(count_result);
    if (count_row && count_row[0])
    {
        total = std::stoi(count_row[0]);
    }
    else
    {
        total = 0;
    }
    mysql_free_result(count_result);

    // 如果没有数据，直接返回空数组
    if (total == 0)
    {
        leaderboard_data = Json::Value(Json::arrayValue);
        return true;
    }

    // 执行查询获取排行榜数据
    std::string query = base_query + time_condition + group_by + order_query + limit_query;

    // 打印查询语句用于调试
    std::cout << "执行排行榜查询: " << query << std::endl;

    MYSQL_RES *result = db->executeQuery(query);

    if (!result)
    {
        error_message = "获取排行榜数据失败，数据库错误";
        std::cout << "查询执行失败" << std::endl;
        return false;
    }

    // 处理查询结果
    MYSQL_ROW row;
    Json::Value users(Json::arrayValue);

    while ((row = mysql_fetch_row(result)))
    {
        Json::Value user;

        int user_id = row[0] ? std::stoi(row[0]) : 0;
        std::string username = row[1] ? row[1] : "";
        std::string avatar = row[2] ? row[2] : "";
        int solved_count = row[3] ? std::stoi(row[3]) : 0;
        int submission_count = row[4] ? std::stoi(row[4]) : 0;
        int score = row[5] ? std::stoi(row[5]) : 0;
        int easy_count = row[6] ? std::stoi(row[6]) : 0;
        int medium_count = row[7] ? std::stoi(row[7]) : 0;
        int hard_count = row[8] ? std::stoi(row[8]) : 0;
        double acceptance_rate = row[9] ? std::stod(row[9]) : 0.0;

        user["user_id"] = user_id;
        user["username"] = username;
        user["avatar"] = avatar;
        user["solved_count"] = solved_count;
        user["submission_count"] = submission_count;
        user["score"] = score;
        user["easy_count"] = easy_count;
        user["medium_count"] = medium_count;
        user["hard_count"] = hard_count;
        user["acceptance_rate"] = acceptance_rate;
        user["is_current_user"] = (user_id == current_user_id);

        users.append(user);
    }

    mysql_free_result(result);
    leaderboard_data = users;

    return true;
}

// 更新用户排行榜统计信息
bool UserService::updateUserLeaderboardStats(int user_id, int problem_id, bool is_accepted)
{
    // 获取数据库连接
    Database *db = Database::getInstance();

    // 1. 获取用户当前统计信息
    User user = getUserInfo(user_id);
    if (user.getId() == 0)
    {
        return false;
    }

    // 2. 获取题目信息，确定难度
    std::string problem_query = "SELECT difficulty FROM problems WHERE id = " + std::to_string(problem_id);
    MYSQL_RES *problem_result = db->executeQuery(problem_query);

    if (!problem_result || mysql_num_rows(problem_result) == 0)
    {
        if (problem_result)
            mysql_free_result(problem_result);
        return false;
    }

    MYSQL_ROW problem_row = mysql_fetch_row(problem_result);
    std::string difficulty = problem_row[0] ? problem_row[0] : "中等";
    mysql_free_result(problem_result);

    // 3. 检查用户是否已经通过该题目，避免重复计算
    std::string check_query = "SELECT COUNT(*) FROM submissions WHERE user_id = " + std::to_string(user_id) +
                              " AND problem_id = " + std::to_string(problem_id) +
                              " AND result = 2"; // 2 表示已通过

    MYSQL_RES *check_result = db->executeQuery(check_query);

    if (!check_result)
    {
        return false;
    }

    MYSQL_ROW check_row = mysql_fetch_row(check_result);
    bool already_solved = check_row[0] && std::stoi(check_row[0]) > 0;
    mysql_free_result(check_result);

    // 4. 更新数据
    std::string update_query = "UPDATE users SET";

    // 总是增加提交次数
    update_query += " submission_count = submission_count + 1";

    // 如果是通过且之前没有通过过，则更新解题数量和积分
    if (is_accepted && !already_solved)
    {
        update_query += ", solved_count = solved_count + 1";

        // 根据难度增加相应难度的题目计数和积分
        if (difficulty == "简单")
        {
            update_query += ", easy_count = easy_count + 1, score = score + 10";
        }
        else if (difficulty == "中等")
        {
            update_query += ", medium_count = medium_count + 1, score = score + 20";
        }
        else if (difficulty == "困难")
        {
            update_query += ", hard_count = hard_count + 1, score = score + 30";
        }
    }

    update_query += " WHERE id = " + std::to_string(user_id);

    // 执行更新并返回结果
    return db->executeCommand(update_query);
}

// ===== 管理员用户管理相关服务方法 =====

// 获取所有用户（支持搜索和筛选）
bool UserService::getAllUsers(int offset, int limit, const std::string& search_term, 
                            int role_filter, int status_filter, Json::Value& user_data, 
                            int& total, std::string& error_message) {
    // 获取数据库连接
    Database* db = Database::getInstance();
    
    // 构建查询条件
    std::string where_clause = "WHERE 1=1 ";
    
    // 添加搜索条件
    if (!search_term.empty()) {
        where_clause += "AND (username LIKE '%" + search_term + "%' OR email LIKE '%" + search_term + "%') ";
    }
    
    // 添加角色筛选
    if (role_filter >= 0 && role_filter <= 2) {
        where_clause += "AND role = " + std::to_string(role_filter) + " ";
    }
    
    // 添加状态筛选
    if (status_filter >= 0 && status_filter <= 2) {
        where_clause += "AND status = " + std::to_string(status_filter) + " ";
    }
    
    // 查询用户总数
    std::string count_query = "SELECT COUNT(*) FROM users " + where_clause;
    MYSQL_RES* count_result = db->executeQuery(count_query);
    
    if (!count_result) {
        error_message = "获取用户列表失败，数据库错误";
        return false;
    }
    
    MYSQL_ROW count_row = mysql_fetch_row(count_result);
    if (!count_row || !count_row[0]) {
        mysql_free_result(count_result);
        error_message = "获取用户列表失败，数据库错误";
        return false;
    }
    
    // 设置总用户数
    total = std::stoi(count_row[0]);
    mysql_free_result(count_result);
    
    // 如果没有用户，直接返回空数组
    if (total == 0) {
        user_data = Json::Value(Json::arrayValue);
        return true;
    }
    
    // 分页查询用户列表
    std::string query = "SELECT * FROM users " + where_clause +
                        "ORDER BY id DESC LIMIT " + std::to_string(limit) + 
                        " OFFSET " + std::to_string(offset);
    
    MYSQL_RES* result = db->executeQuery(query);
    
    if (!result) {
        error_message = "获取用户列表失败，数据库错误";
        return false;
    }
    
    // 处理查询结果
    int num_rows = mysql_num_rows(result);
    user_data = Json::Value(Json::arrayValue);
    
    for (int i = 0; i < num_rows; i++) {
        User user = User::fromDatabaseResult(result, i);
        
        // 转换为JSON对象（不包含敏感信息）
        Json::Value user_obj;
        user_obj["id"] = user.getId();
        user_obj["username"] = user.getUsername();
        user_obj["email"] = user.getEmail();
        user_obj["avatar"] = user.getAvatar();
        user_obj["role"] = static_cast<int>(user.getRole());
        user_obj["status"] = static_cast<int>(user.getStatus());
        user_obj["created_at"] = static_cast<Json::Int64>(user.getCreatedAt());
        user_obj["updated_at"] = static_cast<Json::Int64>(user.getUpdatedAt());
        user_obj["last_login"] = static_cast<Json::Int64>(user.getLastLogin());
        
        user_data.append(user_obj);
    }
    
    mysql_free_result(result);
    return true;
}

// 通过管理员创建用户
bool UserService::createUserByAdmin(const std::string& username, const std::string& password, 
                                  const std::string& email, int role, std::string& error_message) {
    // 验证用户名
    if (!validateUsername(username)) {
        error_message = "用户名格式不正确";
        return false;
    }
    
    // 验证邮箱
    if (!validateEmail(email)) {
        error_message = "邮箱格式不正确";
        return false;
    }
    
    // 验证密码
    if (!validatePassword(password)) {
        error_message = "密码长度不能少于6位";
        return false;
    }
    
    // 验证角色
    if (role < 0 || role > 2) {
        error_message = "角色值无效";
        return false;
    }
    
    // 检查用户名是否已存在
    if (isUserExist(username)) {
        error_message = "用户名已存在";
        return false;
    }
    
    // 检查邮箱是否已存在
    if (isEmailExist(email)) {
        error_message = "邮箱已注册";
        return false;
    }
    
    // 生成密码哈希和盐 - C++11兼容写法
    std::pair<std::string, std::string> password_data = User::generatePasswordHash(password);
    std::string password_hash = password_data.first;
    std::string salt = password_data.second;
    
    // 获取数据库连接
    Database* db = Database::getInstance();
    
    // 准备插入语句
    std::string query = "INSERT INTO users (username, email, password_hash, salt, role, status, created_at, updated_at, "
                        "solved_count, submission_count, score, easy_count, medium_count, hard_count) VALUES (";
    query += "'" + username + "', ";
    query += "'" + email + "', ";
    query += "'" + password_hash + "', ";
    query += "'" + salt + "', ";
    query += std::to_string(role) + ", ";  // 使用管理员指定的角色
    query += std::to_string(static_cast<int>(UserStatus::NORMAL)) + ", ";
    
    time_t now = time(nullptr);
    query += std::to_string(now) + ", ";
    query += std::to_string(now) + ", ";
    
    // 添加排行榜相关字段的初始值
    query += "0, "; // solved_count
    query += "0, "; // submission_count
    query += "0, "; // score
    query += "0, "; // easy_count
    query += "0, "; // medium_count
    query += "0)";  // hard_count
    
    // 执行插入
    bool success = db->executeCommand(query);
    
    if (!success) {
        error_message = "创建用户失败，数据库错误";
        return false;
    }
    
    return true;
}

// 通过管理员更新用户信息
bool UserService::updateUserByAdmin(int user_id, const Json::Value& user_data, 
                                  std::string& error_message) {
    // 检查用户是否存在
    User user = getUserInfo(user_id);
    if (user.getId() == 0) {
        error_message = "用户不存在";
        return false;
    }
    
    // 获取数据库连接
    Database* db = Database::getInstance();
    
    // 构建更新语句
    std::string update_query = "UPDATE users SET ";
    
    // 用户名更新
    if (user_data.isMember("username") && !user_data["username"].asString().empty()) {
        std::string new_username = user_data["username"].asString();
        
        // 如果用户名变更，需要检查是否已存在
        if (new_username != user.getUsername()) {
            if (!validateUsername(new_username)) {
                error_message = "用户名格式不正确";
                return false;
            }
            
            if (isUserExist(new_username)) {
                error_message = "用户名已存在";
                return false;
            }
            
            update_query += "username = '" + new_username + "', ";
        }
    }
    
    // 邮箱更新
    if (user_data.isMember("email") && !user_data["email"].asString().empty()) {
        std::string new_email = user_data["email"].asString();
        
        // 如果邮箱变更，需要检查是否已存在
        if (new_email != user.getEmail()) {
            if (!validateEmail(new_email)) {
                error_message = "邮箱格式不正确";
                return false;
            }
            
            if (isEmailExist(new_email)) {
                error_message = "邮箱已注册";
                return false;
            }
            
            update_query += "email = '" + new_email + "', ";
        }
    }
    
    // 角色更新
    if (user_data.isMember("role")) {
        int new_role = user_data["role"].asInt();
        if (new_role < 0 || new_role > 2) {
            error_message = "角色值无效";
            return false;
        }
        
        update_query += "role = " + std::to_string(new_role) + ", ";
    }
    
    // 状态更新
    if (user_data.isMember("status")) {
        int new_status = user_data["status"].asInt();
        if (new_status < 0 || new_status > 2) {
            error_message = "状态值无效";
            return false;
        }
        
        update_query += "status = " + std::to_string(new_status) + ", ";
    }
    
    // 头像更新
    if (user_data.isMember("avatar") && !user_data["avatar"].asString().empty()) {
        std::string new_avatar = user_data["avatar"].asString();
        update_query += "avatar = '" + new_avatar + "', ";
    }
    
    // 添加更新时间
    time_t now = time(nullptr);
    update_query += "updated_at = " + std::to_string(now);
    
    // 完成更新语句
    update_query += " WHERE id = " + std::to_string(user_id);
    
    // 执行更新
    bool success = db->executeCommand(update_query);
    
    if (!success) {
        error_message = "更新用户信息失败，数据库错误";
        return false;
    }
    
    return true;
}

// 删除用户（软删除）
bool UserService::deleteUser(int user_id, std::string& error_message) {
    // 检查用户是否存在
    User user = getUserInfo(user_id);
    if (user.getId() == 0) {
        error_message = "用户不存在";
        return false;
    }
    
    // 获取数据库连接
    Database* db = Database::getInstance();
    
    // 软删除：更新用户状态为已删除
    std::string query = "UPDATE users SET status = " + 
                        std::to_string(static_cast<int>(UserStatus::DELETED)) + 
                        ", updated_at = " + std::to_string(time(nullptr)) + 
                        " WHERE id = " + std::to_string(user_id);
    
    // 执行更新
    bool success = db->executeCommand(query);
    
    if (!success) {
        error_message = "删除用户失败，数据库错误";
        return false;
    }
    
    return true;
}

// 更改用户角色
bool UserService::changeUserRole(int user_id, UserRole role, std::string& error_message) {
    // 检查用户是否存在
    User user = getUserInfo(user_id);
    if (user.getId() == 0) {
        error_message = "用户不存在";
        return false;
    }
    
    // 验证角色值是否有效
    int role_int = static_cast<int>(role);
    if (role_int < 0 || role_int > 2) {
        error_message = "角色值无效";
        return false;
    }
    
    // 获取数据库连接
    Database* db = Database::getInstance();
    
    // 更新用户角色
    std::string query = "UPDATE users SET role = " + std::to_string(role_int) + 
                        ", updated_at = " + std::to_string(time(nullptr)) + 
                        " WHERE id = " + std::to_string(user_id);
    
    // 执行更新
    bool success = db->executeCommand(query);
    
    if (!success) {
        error_message = "更改用户角色失败，数据库错误";
        return false;
    }
    
    return true;
}

// 重置用户密码
bool UserService::resetUserPassword(int user_id, const std::string& new_password, 
                                  std::string& error_message) {
    // 检查用户是否存在
    User user = getUserInfo(user_id);
    if (user.getId() == 0) {
        error_message = "用户不存在";
        return false;
    }
    
    // 验证新密码强度
    if (!validatePassword(new_password)) {
        error_message = "密码长度不能少于6位";
        return false;
    }
    
    // 生成新的密码哈希和盐 - C++11兼容写法
    std::pair<std::string, std::string> password_data = User::generatePasswordHash(new_password);
    std::string password_hash = password_data.first;
    std::string salt = password_data.second;
    
    // 获取数据库连接
    Database* db = Database::getInstance();
    
    // 更新密码
    std::string query = "UPDATE users SET password_hash = '" + password_hash + 
                        "', salt = '" + salt + 
                        "', updated_at = " + std::to_string(time(nullptr)) + 
                        " WHERE id = " + std::to_string(user_id);
    
    // 执行更新
    bool success = db->executeCommand(query);
    
    if (!success) {
        error_message = "重置密码失败，数据库错误";
        return false;
    }
    
    return true;
}