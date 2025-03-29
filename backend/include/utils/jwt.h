#ifndef JWT_H
#define JWT_H

#include <string>
#include <map>

class JWT {
private:
    static std::string secret_key;
    
    // 编码Base64
    static std::string base64_encode(const std::string& data);
    
    // 解码Base64
    static std::string base64_decode(const std::string& data);
    
    // 创建签名
    static std::string create_signature(const std::string& header, const std::string& payload);

public:
    // 设置密钥
    static void setSecretKey(const std::string& key);
    
    // 生成JWT令牌
    static std::string generateToken(const std::map<std::string, std::string>& payload, long expiry = 3600);
    
    // 验证JWT令牌
    static bool verifyToken(const std::string& token, std::map<std::string, std::string>& payload);
    
    // 从令牌中获取用户ID
    static int getUserIdFromToken(const std::string& token);
    
    // 从令牌中获取用户角色
    static int getUserRoleFromToken(const std::string& token);
};

#endif // JWT_H 