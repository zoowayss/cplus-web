#ifndef DISCUSSION_H
#define DISCUSSION_H

#include <string>
#include <vector>
#include <ctime>

// 讨论帖子类
class Discussion {
private:
    int id;                     // 讨论ID
    int problem_id;             // 关联的题目ID（可选）
    int user_id;                // 发布者ID
    std::string title;          // 标题
    std::string content;        // 内容
    int views;                  // 浏览量
    int likes;                  // 点赞数
    std::time_t created_at;     // 创建时间
    std::time_t updated_at;     // 更新时间

public:
    // 构造函数
    Discussion();
    Discussion(int user_id, const std::string& title, const std::string& content, int problem_id = 0);
    
    // Getters
    int getId() const;
    int getProblemId() const;
    int getUserId() const;
    std::string getTitle() const;
    std::string getContent() const;
    int getViews() const;
    int getLikes() const;
    std::time_t getCreatedAt() const;
    std::time_t getUpdatedAt() const;
    
    // Setters
    void setId(int id);
    void setProblemId(int problem_id);
    void setUserId(int user_id);
    void setTitle(const std::string& title);
    void setContent(const std::string& content);
    void setViews(int views);
    void setLikes(int likes);
    void setCreatedAt(std::time_t created_at);
    void setUpdatedAt(std::time_t updated_at);
    
    // 增加浏览量
    void incrementViews();
    
    // 增加/减少点赞数
    void incrementLikes();
    void decrementLikes();
    
    // 从JSON字符串加载讨论
    static Discussion fromJson(const std::string& json);
    
    // 转换为JSON字符串
    std::string toJson() const;
};

// 讨论回复类
class DiscussionReply {
private:
    int id;                     // 回复ID
    int discussion_id;          // 讨论ID
    int user_id;                // 用户ID
    int parent_id;              // 父回复ID（用于嵌套回复）
    std::string content;        // 内容
    int likes;                  // 点赞数
    std::time_t created_at;     // 创建时间
    std::time_t updated_at;     // 更新时间

public:
    // 构造函数
    DiscussionReply();
    DiscussionReply(int discussion_id, int user_id, const std::string& content, int parent_id = 0);
    
    // Getters
    int getId() const;
    int getDiscussionId() const;
    int getUserId() const;
    int getParentId() const;
    std::string getContent() const;
    int getLikes() const;
    std::time_t getCreatedAt() const;
    std::time_t getUpdatedAt() const;
    
    // Setters
    void setId(int id);
    void setDiscussionId(int discussion_id);
    void setUserId(int user_id);
    void setParentId(int parent_id);
    void setContent(const std::string& content);
    void setLikes(int likes);
    void setCreatedAt(std::time_t created_at);
    void setUpdatedAt(std::time_t updated_at);
    
    // 增加/减少点赞数
    void incrementLikes();
    void decrementLikes();
    
    // 从JSON字符串加载回复
    static DiscussionReply fromJson(const std::string& json);
    
    // 转换为JSON字符串
    std::string toJson() const;
};

// 讨论数据访问对象，处理讨论相关的数据库操作
class DiscussionDAO {
public:
    // 创建讨论
    static bool createDiscussion(Discussion& discussion);
    
    // 通过ID获取讨论
    static Discussion getDiscussionById(int id);
    
    // 更新讨论信息
    static bool updateDiscussion(const Discussion& discussion);
    
    // 删除讨论
    static bool deleteDiscussion(int id);
    
    // 获取所有讨论
    static std::vector<Discussion> getAllDiscussions(int offset = 0, int limit = 10);
    
    // 通过题目ID获取讨论
    static std::vector<Discussion> getDiscussionsByProblemId(int problem_id, int offset = 0, int limit = 10);
    
    // 通过用户ID获取讨论
    static std::vector<Discussion> getDiscussionsByUserId(int user_id, int offset = 0, int limit = 10);
    
    // 获取热门讨论（按浏览量或点赞数排序）
    static std::vector<Discussion> getHotDiscussions(int limit = 10);
    
    // 创建讨论回复
    static bool createDiscussionReply(DiscussionReply& reply);
    
    // 通过ID获取回复
    static DiscussionReply getDiscussionReplyById(int id);
    
    // 更新回复信息
    static bool updateDiscussionReply(const DiscussionReply& reply);
    
    // 删除回复
    static bool deleteDiscussionReply(int id);
    
    // 获取讨论的所有回复
    static std::vector<DiscussionReply> getRepliesByDiscussionId(int discussion_id, int offset = 0, int limit = 20);
    
    // 获取回复的子回复
    static std::vector<DiscussionReply> getChildReplies(int parent_id, int offset = 0, int limit = 10);
    
    // 获取用户的所有回复
    static std::vector<DiscussionReply> getRepliesByUserId(int user_id, int offset = 0, int limit = 10);
};

#endif // DISCUSSION_H 