#include "../../include/models/user.h"
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <sstream>
#include <iomanip>
#include <mysql/mysql.h>
#include <json/json.h>

// 转换为JSON字符串（不包括敏感信息）
std::string User::toJson() const {
    Json::Value root;
    root["id"] = id_;
    root["username"] = username_;
    root["email"] = email_;
    root["avatar"] = avatar_;
    root["role"] = static_cast<int>(role_);
    root["status"] = static_cast<int>(status_);
    root["created_at"] = static_cast<Json::Int64>(created_at_);
    root["updated_at"] = static_cast<Json::Int64>(updated_at_);
    root["last_login"] = static_cast<Json::Int64>(last_login_);
    
    // 不包含敏感信息如密码哈希和盐
    
    Json::StreamWriterBuilder writer;
    return Json::writeString(writer, root);
}

// 从数据库结果集创建用户对象
User User::fromDatabaseResult(void* result, int row) {
    MYSQL_RES* res = static_cast<MYSQL_RES*>(result);
    MYSQL_ROW mysql_row = mysql_fetch_row(res);
    
    if (!mysql_row) {
        return User();
    }
    
    // 移动到特定行
    mysql_data_seek(res, row);
    mysql_row = mysql_fetch_row(res);
    
    if (!mysql_row) {
        return User();
    }
    
    User user;
    
    if (mysql_row[0]) user.setId(std::stoi(mysql_row[0]));
    if (mysql_row[1]) user.setUsername(mysql_row[1]);
    if (mysql_row[2]) user.setEmail(mysql_row[2]);
    if (mysql_row[3]) user.setPasswordHash(mysql_row[3]);
    if (mysql_row[4]) user.setSalt(mysql_row[4]);
    if (mysql_row[5]) user.setAvatar(mysql_row[5]);
    if (mysql_row[6]) user.setRole(static_cast<UserRole>(std::stoi(mysql_row[6])));
    if (mysql_row[7]) user.setStatus(static_cast<UserStatus>(std::stoi(mysql_row[7])));
    if (mysql_row[8]) user.setCreatedAt(std::stoull(mysql_row[8]));
    if (mysql_row[9]) user.setUpdatedAt(std::stoull(mysql_row[9]));
    if (mysql_row[10]) user.setLastLogin(std::stoull(mysql_row[10]));
    
    return user;
}

// 生成随机盐
static std::string generateSalt(size_t length = 16) {
    std::vector<unsigned char> salt(length);
    RAND_bytes(salt.data(), length);
    
    std::stringstream ss;
    for (auto byte : salt) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    
    return ss.str();
}

// 生成密码哈希
std::pair<std::string, std::string> User::generatePasswordHash(const std::string& password) {
    std::string salt = generateSalt();
    
    // 将密码和盐组合
    std::string salted_password = password + salt;
    
    // 计算SHA-256哈希
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(salted_password.c_str()), 
           salted_password.length(), hash);
    
    // 转换为十六进制字符串
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    return {ss.str(), salt};
}

// 检查密码是否匹配
bool User::checkPassword(const std::string& password) const {
    // 将输入的密码和存储的盐组合
    std::string salted_password = password + salt_;
    
    // 计算SHA-256哈希
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(salted_password.c_str()), 
           salted_password.length(), hash);
    
    // 转换为十六进制字符串
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    // 比较哈希值
    return ss.str() == password_hash_;
} 