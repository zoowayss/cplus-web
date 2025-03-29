#include "../../include/utils/jwt.h"
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <ctime>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <json/json.h>

// 初始化密钥
std::string JWT::secret_key = "cplus_web_secret_key_for_jwt_token";

// Base64编码
std::string JWT::base64_encode(const std::string& data) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, data.c_str(), data.length());
    BIO_flush(b64);
    
    BUF_MEM* bptr;
    BIO_get_mem_ptr(b64, &bptr);
    
    std::string result(bptr->data, bptr->length - 1); // 减1是为了去掉换行符
    BIO_free_all(b64);
    
    // 替换URL安全的Base64字符
    std::replace(result.begin(), result.end(), '+', '-');
    std::replace(result.begin(), result.end(), '/', '_');
    std::replace(result.begin(), result.end(), '=', ' ');
    
    // 去除尾部空格
    result.erase(result.find_last_not_of(' ') + 1);
    
    return result;
}

// Base64解码
std::string JWT::base64_decode(const std::string& data) {
    std::string base64 = data;
    
    // 替换回标准Base64字符
    std::replace(base64.begin(), base64.end(), '-', '+');
    std::replace(base64.begin(), base64.end(), '_', '/');
    
    // 添加适当的填充
    while (base64.length() % 4 != 0) {
        base64.push_back('=');
    }
    
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bmem = BIO_new_mem_buf(base64.c_str(), base64.length());
    bmem = BIO_push(b64, bmem);
    
    std::vector<char> buffer(base64.length());
    int decoded_length = BIO_read(bmem, buffer.data(), base64.length());
    BIO_free_all(bmem);
    
    return std::string(buffer.data(), decoded_length);
}

// 创建HMAC签名
std::string JWT::create_signature(const std::string& header, const std::string& payload) {
    std::string data = header + "." + payload;
    
    unsigned char hmac[EVP_MAX_MD_SIZE];
    unsigned int hmac_len;
    
    HMAC(EVP_sha256(), secret_key.c_str(), secret_key.length(),
         reinterpret_cast<const unsigned char*>(data.c_str()), data.length(),
         hmac, &hmac_len);
    
    return std::string(reinterpret_cast<char*>(hmac), hmac_len);
}

// 设置密钥
void JWT::setSecretKey(const std::string& key) {
    secret_key = key;
}

// 生成JWT Token
std::string JWT::generateToken(const std::map<std::string, std::string>& payload_data, long expiry) {
    // 构建头部
    Json::Value header;
    header["alg"] = "HS256";
    header["typ"] = "JWT";
    
    Json::StreamWriterBuilder writer_builder;
    writer_builder["indentation"] = "";
    std::string header_str = Json::writeString(writer_builder, header);
    
    // 构建负载
    Json::Value payload;
    for (const auto& item : payload_data) {
        payload[item.first] = item.second;
    }
    
    // 添加过期时间
    time_t now = time(nullptr);
    payload["exp"] = std::to_string(now + expiry);
    payload["iat"] = std::to_string(now);
    
    std::string payload_str = Json::writeString(writer_builder, payload);
    
    // 编码头部和负载
    std::string encoded_header = base64_encode(header_str);
    std::string encoded_payload = base64_encode(payload_str);
    
    // 创建签名
    std::string signature = create_signature(encoded_header, encoded_payload);
    std::string encoded_signature = base64_encode(signature);
    
    // 组合JWT
    return encoded_header + "." + encoded_payload + "." + encoded_signature;
}

// 验证JWT Token
bool JWT::verifyToken(const std::string& token, std::map<std::string, std::string>& out_payload) {
    // 分割Token
    size_t first_dot = token.find('.');
    size_t last_dot = token.rfind('.');
    
    if (first_dot == std::string::npos || last_dot == std::string::npos || first_dot == last_dot) {
        return false;
    }
    
    std::string encoded_header = token.substr(0, first_dot);
    std::string encoded_payload = token.substr(first_dot + 1, last_dot - first_dot - 1);
    std::string encoded_signature = token.substr(last_dot + 1);
    
    // 验证签名
    std::string expected_signature = create_signature(encoded_header, encoded_payload);
    std::string actual_signature = base64_decode(encoded_signature);
    
    if (expected_signature != actual_signature) {
        return false;
    }
    
    // 解析负载
    std::string payload_str = base64_decode(encoded_payload);
    
    Json::CharReaderBuilder reader_builder;
    Json::Value payload;
    std::istringstream payload_stream(payload_str);
    std::string errors;
    
    if (!Json::parseFromStream(reader_builder, payload_stream, &payload, &errors)) {
        return false;
    }
    
    // 检查令牌是否过期
    if (payload.isMember("exp")) {
        long exp_time = std::stol(payload["exp"].asString());
        time_t now = time(nullptr);
        
        if (now > exp_time) {
            return false;
        }
    }
    
    // 提取负载数据
    for (const auto& key : payload.getMemberNames()) {
        out_payload[key] = payload[key].asString();
    }
    
    return true;
}

// 从令牌中获取用户ID
int JWT::getUserIdFromToken(const std::string& token) {
    std::map<std::string, std::string> payload;
    if (verifyToken(token, payload)) {
        if (payload.find("user_id") != payload.end()) {
            return std::stoi(payload["user_id"]);
        }
    }
    return -1;
}

// 从令牌中获取用户角色
int JWT::getUserRoleFromToken(const std::string& token) {
    std::map<std::string, std::string> payload;
    if (verifyToken(token, payload)) {
        if (payload.find("role") != payload.end()) {
            return std::stoi(payload["role"]);
        }
    }
    return -1;
} 