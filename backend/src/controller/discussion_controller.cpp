#include "../../include/controller/discussion_controller.h"
#include <json/json.h>
#include <iostream>
#include <regex>
#include <sstream>

// 注册路由
void DiscussionController::registerRoutes(http::HttpServer* server) {
    // 获取所有讨论
    server->get("/api/discussions", [this](const http::Request& req, http::Response& res) {
        this->handleGetAllDiscussions(req, res);
    });
    
    // 获取题目相关讨论
    server->get("/api/problems/[0-9]+/discussions", [this](const http::Request& req, http::Response& res) {
        this->handleGetProblemDiscussions(req, res);
    });
    
    // 获取讨论详情
    server->get("/api/discussions/[0-9]+", [this](const http::Request& req, http::Response& res) {
        this->handleGetDiscussionDetail(req, res);
    });
    
    // 创建讨论
    server->post("/api/discussions", middleware::AuthMiddleware::protect([this](const http::Request& req, http::Response& res) {
        this->handleCreateDiscussion(req, res);
    }));
    
    // 更新讨论
    server->put("/api/discussions/[0-9]+", middleware::AuthMiddleware::protect([this](const http::Request& req, http::Response& res) {
        this->handleUpdateDiscussion(req, res);
    }));
    
    // 删除讨论
    server->del("/api/discussions/[0-9]+", middleware::AuthMiddleware::protect([this](const http::Request& req, http::Response& res) {
        this->handleDeleteDiscussion(req, res);
    }));
    
    // 获取讨论回复
    server->get("/api/discussions/[0-9]+/replies", [this](const http::Request& req, http::Response& res) {
        this->handleGetReplies(req, res);
    });
    
    // 创建回复
    server->post("/api/discussions/[0-9]+/replies", middleware::AuthMiddleware::protect([this](const http::Request& req, http::Response& res) {
        this->handleCreateReply(req, res);
    }));
    
    // 删除回复
    server->del("/api/replies/[0-9]+", middleware::AuthMiddleware::protect([this](const http::Request& req, http::Response& res) {
        this->handleDeleteReply(req, res);
    }));
}

// 解析查询参数
std::map<std::string, std::string> parseQueryParams(const std::string& query_string) {
    std::map<std::string, std::string> params;
    std::istringstream iss(query_string);
    std::string token;
    
    // 按 & 分割查询字符串
    while (std::getline(iss, token, '&')) {
        std::size_t pos = token.find('=');
        if (pos != std::string::npos) {
            std::string key = token.substr(0, pos);
            std::string value = token.substr(pos + 1);
            params[key] = value;
        }
    }
    
    return params;
}

// 获取所有讨论
void DiscussionController::handleGetAllDiscussions(const http::Request& req, http::Response& res) {
    // 解析分页参数
    int offset = 0, limit = 10;
    
    // 解析查询参数
    size_t query_pos = req.path.find('?');
    if (query_pos != std::string::npos) {
        std::string query_string = req.path.substr(query_pos + 1);
        auto params = parseQueryParams(query_string);
        
        // 尝试获取偏移量参数
        auto offset_it = params.find("offset");
        if (offset_it != params.end()) {
            try {
                offset = std::stoi(offset_it->second);
            } catch (std::exception& e) {
                // 忽略无效参数
            }
        }
        
        // 尝试获取限制参数
        auto limit_it = params.find("limit");
        if (limit_it != params.end()) {
            try {
                limit = std::stoi(limit_it->second);
                // 限制最大查询数量为50
                if (limit > 50) limit = 50;
            } catch (std::exception& e) {
                // 忽略无效参数
            }
        }
    }
    
    // 获取讨论列表
    std::vector<Discussion> discussions = DiscussionDAO::getAllDiscussions(offset, limit);
    
    // 构建响应
    Json::Value discussionsJson(Json::arrayValue);
    for (const auto& discussion : discussions) {
        Json::Value discussionJson;
        Json::Reader reader;
        reader.parse(discussion.toJson(), discussionJson);
        discussionsJson.append(discussionJson);
    }
    
    Json::Value data;
    data["discussions"] = discussionsJson;
    data["total"] = static_cast<int>(discussions.size());
    data["offset"] = offset;
    data["limit"] = limit;
    
    sendSuccessResponse(res, "获取讨论列表成功", data);
}

// 获取题目相关讨论
void DiscussionController::handleGetProblemDiscussions(const http::Request& req, http::Response& res) {
    // 从URL中提取题目ID
    int problem_id = getIdFromPath(req.path, "/api/problems/");
    if (problem_id <= 0) {
        sendErrorResponse(res, "无效的题目ID", 400);
        return;
    }
    
    // 解析分页参数
    int offset = 0, limit = 10;
    
    // 解析查询参数
    size_t query_pos = req.path.find('?');
    if (query_pos != std::string::npos) {
        std::string query_string = req.path.substr(query_pos + 1);
        auto params = parseQueryParams(query_string);
        
        // 尝试获取偏移量参数
        auto offset_it = params.find("offset");
        if (offset_it != params.end()) {
            try {
                offset = std::stoi(offset_it->second);
            } catch (std::exception& e) {
                // 忽略无效参数
            }
        }
        
        // 尝试获取限制参数
        auto limit_it = params.find("limit");
        if (limit_it != params.end()) {
            try {
                limit = std::stoi(limit_it->second);
                // 限制最大查询数量为50
                if (limit > 50) limit = 50;
            } catch (std::exception& e) {
                // 忽略无效参数
            }
        }
    }
    
    // 获取题目相关讨论
    std::vector<Discussion> discussions = DiscussionDAO::getDiscussionsByProblemId(problem_id, offset, limit);
    
    // 构建响应
    Json::Value discussionsJson(Json::arrayValue);
    for (const auto& discussion : discussions) {
        Json::Value discussionJson;
        Json::Reader reader;
        reader.parse(discussion.toJson(), discussionJson);
        discussionsJson.append(discussionJson);
    }
    
    Json::Value data;
    data["discussions"] = discussionsJson;
    data["problem_id"] = problem_id;
    data["total"] = static_cast<int>(discussions.size());
    data["offset"] = offset;
    data["limit"] = limit;
    
    sendSuccessResponse(res, "获取题目讨论成功", data);
}

// 获取讨论详情
void DiscussionController::handleGetDiscussionDetail(const http::Request& req, http::Response& res) {
    // 从URL中提取讨论ID
    int discussion_id = getIdFromPath(req.path, "/api/discussions/");
    if (discussion_id <= 0) {
        sendErrorResponse(res, "无效的讨论ID", 400);
        return;
    }
    
    // 获取讨论详情
    Discussion discussion = DiscussionDAO::getDiscussionById(discussion_id);
    
    if (discussion.getId() == 0) {
        sendErrorResponse(res, "讨论不存在", 404);
        return;
    }
    
    // 增加浏览量
    discussion.incrementViews();
    DiscussionDAO::updateDiscussion(discussion);
    
    // 转换为JSON
    Json::Value discussionJson;
    Json::Reader reader;
    reader.parse(discussion.toJson(), discussionJson);
    
    // 构建响应
    Json::Value data;
    data["discussion"] = discussionJson;
    
    sendSuccessResponse(res, "获取讨论详情成功", data);
}

// 创建讨论
void DiscussionController::handleCreateDiscussion(const http::Request& req, http::Response& res) {
    // 从请求头获取令牌获取用户ID
    std::string auth_header = req.get_header("Authorization");
    std::string token = auth_header.substr(7);
    int user_id = JWT::getUserIdFromToken(token);
    
    if (user_id <= 0) {
        sendErrorResponse(res, "无效的用户ID", 401);
        return;
    }
    
    // 解析请求体
    Json::Value reqJson = parseRequestBody(req.body);
    
    if (!reqJson.isMember("title") || !reqJson.isMember("content")) {
        sendErrorResponse(res, "请提供标题和内容", 400);
        return;
    }
    
    // 创建讨论对象
    std::string title = reqJson["title"].asString();
    std::string content = reqJson["content"].asString();
    int problem_id = reqJson.isMember("problem_id") ? reqJson["problem_id"].asInt() : 0;
    
    Discussion discussion(user_id, title, content, problem_id);
    
    // 保存讨论
    if (DiscussionDAO::createDiscussion(discussion)) {
        // 构建响应
        Json::Value data;
        Json::Value discussionJson;
        Json::Reader reader;
        reader.parse(discussion.toJson(), discussionJson);
        data["discussion"] = discussionJson;
        
        sendSuccessResponse(res, "创建讨论成功", data);
    } else {
        sendErrorResponse(res, "创建讨论失败", 500);
    }
}

// 更新讨论
void DiscussionController::handleUpdateDiscussion(const http::Request& req, http::Response& res) {
    // 从URL中提取讨论ID
    int discussion_id = getIdFromPath(req.path, "/api/discussions/");
    if (discussion_id <= 0) {
        sendErrorResponse(res, "无效的讨论ID", 400);
        return;
    }
    
    // 从请求头获取令牌获取用户ID
    std::string auth_header = req.get_header("Authorization");
    std::string token = auth_header.substr(7);
    int user_id = JWT::getUserIdFromToken(token);
    int user_role = JWT::getUserRoleFromToken(token);
    
    if (user_id <= 0) {
        sendErrorResponse(res, "无效的用户ID", 401);
        return;
    }
    
    // 获取讨论
    Discussion discussion = DiscussionDAO::getDiscussionById(discussion_id);
    
    if (discussion.getId() == 0) {
        sendErrorResponse(res, "讨论不存在", 404);
        return;
    }
    
    // 检查权限
    if (discussion.getUserId() != user_id && user_role < 2) {
        sendErrorResponse(res, "没有权限修改此讨论", 403);
        return;
    }
    
    // 解析请求体
    Json::Value reqJson = parseRequestBody(req.body);
    
    // 更新讨论对象
    if (reqJson.isMember("title")) {
        discussion.setTitle(reqJson["title"].asString());
    }
    
    if (reqJson.isMember("content")) {
        discussion.setContent(reqJson["content"].asString());
    }
    
    // 更新时间
    discussion.setUpdatedAt(std::time(nullptr));
    
    // 保存更新
    if (DiscussionDAO::updateDiscussion(discussion)) {
        // 构建响应
        Json::Value data;
        Json::Value discussionJson;
        Json::Reader reader;
        reader.parse(discussion.toJson(), discussionJson);
        data["discussion"] = discussionJson;
        
        sendSuccessResponse(res, "更新讨论成功", data);
    } else {
        sendErrorResponse(res, "更新讨论失败", 500);
    }
}

// 删除讨论
void DiscussionController::handleDeleteDiscussion(const http::Request& req, http::Response& res) {
    // 从URL中提取讨论ID
    int discussion_id = getIdFromPath(req.path, "/api/discussions/");
    if (discussion_id <= 0) {
        sendErrorResponse(res, "无效的讨论ID", 400);
        return;
    }
    
    // 从请求头获取令牌获取用户ID
    std::string auth_header = req.get_header("Authorization");
    std::string token = auth_header.substr(7);
    int user_id = JWT::getUserIdFromToken(token);
    int user_role = JWT::getUserRoleFromToken(token);
    
    if (user_id <= 0) {
        sendErrorResponse(res, "无效的用户ID", 401);
        return;
    }
    
    // 获取讨论
    Discussion discussion = DiscussionDAO::getDiscussionById(discussion_id);
    
    if (discussion.getId() == 0) {
        sendErrorResponse(res, "讨论不存在", 404);
        return;
    }
    
    // 检查权限
    if (discussion.getUserId() != user_id && user_role < 2) {
        sendErrorResponse(res, "没有权限删除此讨论", 403);
        return;
    }
    
    // 删除讨论
    if (DiscussionDAO::deleteDiscussion(discussion_id)) {
        sendSuccessResponse(res, "删除讨论成功");
    } else {
        sendErrorResponse(res, "删除讨论失败", 500);
    }
}

// 获取讨论回复
void DiscussionController::handleGetReplies(const http::Request& req, http::Response& res) {
    // 从URL中提取讨论ID
    int discussion_id = getIdFromPath(req.path, "/api/discussions/");
    if (discussion_id <= 0) {
        sendErrorResponse(res, "无效的讨论ID", 400);
        return;
    }
    
    // 解析分页参数
    int offset = 0, limit = 20;
    
    // 解析查询参数
    size_t query_pos = req.path.find('?');
    if (query_pos != std::string::npos) {
        std::string query_string = req.path.substr(query_pos + 1);
        auto params = parseQueryParams(query_string);
        
        // 尝试获取偏移量参数
        auto offset_it = params.find("offset");
        if (offset_it != params.end()) {
            try {
                offset = std::stoi(offset_it->second);
            } catch (std::exception& e) {
                // 忽略无效参数
            }
        }
        
        // 尝试获取限制参数
        auto limit_it = params.find("limit");
        if (limit_it != params.end()) {
            try {
                limit = std::stoi(limit_it->second);
                // 限制最大查询数量
                if (limit > 100) limit = 100;
            } catch (std::exception& e) {
                // 忽略无效参数
            }
        }
    }
    
    // 获取回复
    std::vector<DiscussionReply> replies = DiscussionDAO::getRepliesByDiscussionId(discussion_id, offset, limit);
    
    // 构建响应
    Json::Value repliesJson(Json::arrayValue);
    for (const auto& reply : replies) {
        Json::Value replyJson;
        Json::Reader reader;
        reader.parse(reply.toJson(), replyJson);
        
        // 获取子回复
        std::vector<DiscussionReply> childReplies = DiscussionDAO::getChildReplies(reply.getId(), 0, 10);
        if (!childReplies.empty()) {
            Json::Value childRepliesJson(Json::arrayValue);
            for (const auto& childReply : childReplies) {
                Json::Value childReplyJson;
                reader.parse(childReply.toJson(), childReplyJson);
                childRepliesJson.append(childReplyJson);
            }
            replyJson["child_replies"] = childRepliesJson;
        }
        
        repliesJson.append(replyJson);
    }
    
    Json::Value data;
    data["replies"] = repliesJson;
    data["discussion_id"] = discussion_id;
    
    sendSuccessResponse(res, "获取回复成功", data);
}

// 创建回复
void DiscussionController::handleCreateReply(const http::Request& req, http::Response& res) {
    // 从URL中提取讨论ID
    int discussion_id = getIdFromPath(req.path, "/api/discussions/");
    if (discussion_id <= 0) {
        sendErrorResponse(res, "无效的讨论ID", 400);
        return;
    }
    
    // 从请求头获取令牌获取用户ID
    std::string auth_header = req.get_header("Authorization");
    std::string token = auth_header.substr(7);
    int user_id = JWT::getUserIdFromToken(token);
    
    if (user_id <= 0) {
        sendErrorResponse(res, "无效的用户ID", 401);
        return;
    }
    
    // 解析请求体
    Json::Value reqJson = parseRequestBody(req.body);
    
    if (!reqJson.isMember("content")) {
        sendErrorResponse(res, "请提供回复内容", 400);
        return;
    }
    
    // 创建回复对象
    std::string content = reqJson["content"].asString();
    int parent_id = reqJson.isMember("parent_id") ? reqJson["parent_id"].asInt() : 0;
    
    DiscussionReply reply(discussion_id, user_id, content, parent_id);
    
    // 保存回复
    if (DiscussionDAO::createDiscussionReply(reply)) {
        // 构建响应
        Json::Value data;
        Json::Value replyJson;
        Json::Reader reader;
        reader.parse(reply.toJson(), replyJson);
        data["reply"] = replyJson;
        
        sendSuccessResponse(res, "创建回复成功", data);
    } else {
        sendErrorResponse(res, "创建回复失败", 500);
    }
}

// 删除回复
void DiscussionController::handleDeleteReply(const http::Request& req, http::Response& res) {
    // 从URL中提取回复ID
    int reply_id = getIdFromPath(req.path, "/api/replies/");
    if (reply_id <= 0) {
        sendErrorResponse(res, "无效的回复ID", 400);
        return;
    }
    
    // 从请求头获取令牌获取用户ID
    std::string auth_header = req.get_header("Authorization");
    std::string token = auth_header.substr(7);
    int user_id = JWT::getUserIdFromToken(token);
    int user_role = JWT::getUserRoleFromToken(token);
    
    if (user_id <= 0) {
        sendErrorResponse(res, "无效的用户ID", 401);
        return;
    }
    
    // 获取回复
    DiscussionReply reply = DiscussionDAO::getDiscussionReplyById(reply_id);
    
    if (reply.getId() == 0) {
        sendErrorResponse(res, "回复不存在", 404);
        return;
    }
    
    // 检查权限
    if (reply.getUserId() != user_id && user_role < 2) {
        sendErrorResponse(res, "没有权限删除此回复", 403);
        return;
    }
    
    // 删除回复
    if (DiscussionDAO::deleteDiscussionReply(reply_id)) {
        sendSuccessResponse(res, "删除回复成功");
    } else {
        sendErrorResponse(res, "删除回复失败", 500);
    }
}

// 从URL路径获取ID
int DiscussionController::getIdFromPath(const std::string& path, const std::string& prefix) {
    std::regex pattern(prefix + "([0-9]+)");
    std::smatch matches;
    
    if (std::regex_search(path, matches, pattern) && matches.size() > 1) {
        try {
            return std::stoi(matches[1].str());
        } catch (std::exception& e) {
            return -1;
        }
    }
    
    return -1;
} 