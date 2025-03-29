#ifndef BASE_CONTROLLER_H
#define BASE_CONTROLLER_H

#include "../http/http_server.h"
#include <json/json.h>
#include <string>
#include <map>

// 从请求体解析JSON的通用方法
Json::Value parseRequestBody(const std::string& body);

// 将JSON对象序列化为字符串的通用方法
std::string jsonToString(const Json::Value& value);

class BaseController {
public:
    // 注册路由方法（将在派生类中实现）
    virtual void registerRoutes(http::HttpServer* server) = 0;
    
    // 析构函数声明为虚函数
    virtual ~BaseController() = default;
    
protected:
    // 成功响应辅助方法
    void sendSuccessResponse(http::Response& res, const std::string& message, const Json::Value& data = Json::Value());
    
    // 错误响应辅助方法
    void sendErrorResponse(http::Response& res, const std::string& message, int statusCode = 400);
};

#endif // BASE_CONTROLLER_H 