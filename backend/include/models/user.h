#ifndef USER_H
#define USER_H

#include <string>
#include <map>
#include <ctime>
#include <vector>

// 用户状态枚举
enum class UserStatus {
    NORMAL = 0,   // 正常
    BANNED = 1,   // 禁用
    DELETED = 2   // 已删除
};

// 用户角色枚举
enum class UserRole {
    STUDENT = 0,  // 学生
    TEACHER = 1,  // 教师
    ADMIN = 2     // 管理员
};

// 用户模型
class User {
private:
    int id_;
    std::string username_;
    std::string email_;
    std::string avatar_;
    std::string password_hash_;
    std::string salt_;
    UserRole role_;
    UserStatus status_;
    time_t created_at_;
    time_t updated_at_;
    time_t last_login_;

public:
    // 构造函数
    User() : id_(0), role_(UserRole::STUDENT), status_(UserStatus::NORMAL), 
             created_at_(0), updated_at_(0), last_login_(0) {}
    
    // 带参数的构造函数
    User(int id, const std::string& username, const std::string& email, 
         const std::string& password_hash, const std::string& salt, 
         UserRole role = UserRole::STUDENT, UserStatus status = UserStatus::NORMAL) 
        : id_(id), username_(username), email_(email), password_hash_(password_hash), 
          salt_(salt), role_(role), status_(status) {
        time_t now = time(nullptr);
        created_at_ = now;
        updated_at_ = now;
        last_login_ = 0;
    }
    
    // Getters
    int getId() const { return id_; }
    std::string getUsername() const { return username_; }
    std::string getEmail() const { return email_; }
    std::string getAvatar() const { return avatar_; }
    std::string getPasswordHash() const { return password_hash_; }
    std::string getSalt() const { return salt_; }
    UserRole getRole() const { return role_; }
    UserStatus getStatus() const { return status_; }
    time_t getCreatedAt() const { return created_at_; }
    time_t getUpdatedAt() const { return updated_at_; }
    time_t getLastLogin() const { return last_login_; }
    
    // Setters
    void setId(int id) { id_ = id; }
    void setUsername(const std::string& username) { username_ = username; }
    void setEmail(const std::string& email) { email_ = email; }
    void setAvatar(const std::string& avatar) { avatar_ = avatar; }
    void setPasswordHash(const std::string& password_hash) { password_hash_ = password_hash; }
    void setSalt(const std::string& salt) { salt_ = salt; }
    void setRole(UserRole role) { role_ = role; }
    void setStatus(UserStatus status) { status_ = status; }
    void setCreatedAt(time_t created_at) { created_at_ = created_at; }
    void setUpdatedAt(time_t updated_at) { updated_at_ = updated_at; }
    void setLastLogin(time_t last_login) { last_login_ = last_login; }
    
    // 更新最后登录时间
    void updateLastLogin() { 
        last_login_ = time(nullptr);
        updated_at_ = last_login_;
    }
    
    // 转换为JSON字符串（不包括敏感信息）
    std::string toJson() const;
    
    // 从数据库结果集创建用户对象
    static User fromDatabaseResult(void* result, int row);
    
    // 检查密码是否匹配
    bool checkPassword(const std::string& password) const;
    
    // 生成密码哈希和盐
    static std::pair<std::string, std::string> generatePasswordHash(const std::string& password);
};

// 用户数据访问对象，处理用户相关的数据库操作
class UserDAO {
public:
    // 创建用户
    static bool createUser(User& user);
    
    // 通过ID获取用户
    static User getUserById(int id);
    
    // 通过用户名获取用户
    static User getUserByUsername(const std::string& username);
    
    // 通过邮箱获取用户
    static User getUserByEmail(const std::string& email);
    
    // 更新用户信息
    static bool updateUser(const User& user);
    
    // 删除用户
    static bool deleteUser(int id);
    
    // 获取所有用户
    static std::vector<User> getAllUsers(int offset = 0, int limit = 10);
    
    // 获取用户总数
    static int getUserCount();
    
    // 验证用户登录
    static User validateLogin(const std::string& username, const std::string& password);
};

#endif // USER_H 