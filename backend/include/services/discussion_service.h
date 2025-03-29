#ifndef DISCUSSION_SERVICE_H
#define DISCUSSION_SERVICE_H

#include <string>
#include <vector>
#include "../models/discussion.h"

class DiscussionService {
public:
    // 创建讨论
    static bool createDiscussion(Discussion& discussion, std::string& error_message);
    
    // 获取讨论信息
    static Discussion getDiscussionInfo(int discussion_id);
    
    // 更新讨论
    static bool updateDiscussion(const Discussion& discussion, int user_id, std::string& error_message);
    
    // 删除讨论
    static bool deleteDiscussion(int discussion_id, int user_id, std::string& error_message);
    
    // 获取所有讨论
    static std::vector<Discussion> getAllDiscussions(int offset = 0, int limit = 10);
    
    // 获取题目相关讨论
    static std::vector<Discussion> getProblemDiscussions(int problem_id, int offset = 0, int limit = 10);
    
    // 获取用户创建的讨论
    static std::vector<Discussion> getUserDiscussions(int user_id, int offset = 0, int limit = 10);
    
    // 获取热门讨论
    static std::vector<Discussion> getHotDiscussions(int limit = 10);
    
    // 添加回复
    static bool addReply(DiscussionReply& reply, std::string& error_message);
    
    // 更新回复
    static bool updateReply(const DiscussionReply& reply, int user_id, std::string& error_message);
    
    // 删除回复
    static bool deleteReply(int reply_id, int user_id, std::string& error_message);
    
    // 获取讨论的所有回复
    static std::vector<DiscussionReply> getDiscussionReplies(int discussion_id, int offset = 0, int limit = 20);
    
    // 获取回复的子回复
    static std::vector<DiscussionReply> getChildReplies(int parent_id, int offset = 0, int limit = 10);
    
    // 检查用户是否有权限操作讨论
    static bool checkDiscussionPermission(int discussion_id, int user_id);
    
    // 检查用户是否有权限操作回复
    static bool checkReplyPermission(int reply_id, int user_id);
    
    // 增加讨论浏览量
    static bool incrementDiscussionViews(int discussion_id);
    
    // 点赞/取消点赞讨论
    static bool toggleDiscussionLike(int discussion_id, int user_id, bool is_like);
    
    // 点赞/取消点赞回复
    static bool toggleReplyLike(int reply_id, int user_id, bool is_like);

private:
    // 验证讨论数据
    static bool validateDiscussion(const Discussion& discussion, std::string& error_message);
    
    // 验证回复数据
    static bool validateReply(const DiscussionReply& reply, std::string& error_message);
};

#endif // DISCUSSION_SERVICE_H 