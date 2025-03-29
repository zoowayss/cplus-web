#ifndef DISCUSSION_CONTROLLER_H
#define DISCUSSION_CONTROLLER_H

#include "../http/http_server.h"
#include "../models/discussion.h"
#include "../middleware/auth_middleware.h"
#include "base_controller.h"

// 讨论控制器类
class DiscussionController : public BaseController {
public:
    // 构造函数
    DiscussionController() {}
    
    // 注册路由
    void registerRoutes(http::HttpServer* server) override;
    
private:
    // 处理方法
    
    // 获取所有讨论
    void handleGetAllDiscussions(const http::Request& req, http::Response& res);
    
    // 获取题目相关讨论
    void handleGetProblemDiscussions(const http::Request& req, http::Response& res);
    
    // 获取讨论详情
    void handleGetDiscussionDetail(const http::Request& req, http::Response& res);
    
    // 创建讨论
    void handleCreateDiscussion(const http::Request& req, http::Response& res);
    
    // 更新讨论
    void handleUpdateDiscussion(const http::Request& req, http::Response& res);
    
    // 删除讨论
    void handleDeleteDiscussion(const http::Request& req, http::Response& res);
    
    // 获取讨论回复
    void handleGetReplies(const http::Request& req, http::Response& res);
    
    // 创建回复
    void handleCreateReply(const http::Request& req, http::Response& res);
    
    // 删除回复
    void handleDeleteReply(const http::Request& req, http::Response& res);
    
    // 从URL路径获取ID
    int getIdFromPath(const std::string& path, const std::string& prefix);
};

#endif // DISCUSSION_CONTROLLER_H 