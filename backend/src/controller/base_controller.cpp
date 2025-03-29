#include "../../include/controller/base_controller.h"
#include <sstream>

// 从请求体解析JSON
Json::Value parseRequestBody(const std::string& body) {
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;
    std::istringstream iss(body);
    
    if (!Json::parseFromStream(builder, iss, &root, &errors)) {
        return Json::Value();
    }
    
    return root;
}

// 将JSON对象序列化为字符串（使用UTF-8编码）
std::string jsonToString(const Json::Value& value) {
    Json::StreamWriterBuilder builder;
    // 启用UTF-8直接输出
    builder["emitUTF8"] = true;
    // 使用制表符缩进，美化输出
    builder["indentation"] = "\t";
    return Json::writeString(builder, value);
}

// 成功响应辅助方法
void BaseController::sendSuccessResponse(http::Response& res, const std::string& message, const Json::Value& data) {
    Json::Value response;
    response["status"] = "ok";
    response["message"] = message;
    
    // 如果提供了数据，添加到响应中
    if (!data.isNull()) {
        for (const auto& key : data.getMemberNames()) {
            response[key] = data[key];
        }
    }
    
    res.json(jsonToString(response));
}

// 错误响应辅助方法
void BaseController::sendErrorResponse(http::Response& res, const std::string& message, int statusCode) {
    res.status_code = statusCode;
    res.status_message = statusCode == 401 ? "Unauthorized" : 
                         statusCode == 403 ? "Forbidden" :
                         statusCode == 404 ? "Not Found" : "Bad Request";
    
    Json::Value response;
    response["status"] = "error";
    response["message"] = message;
    
    res.json(jsonToString(response));
} 