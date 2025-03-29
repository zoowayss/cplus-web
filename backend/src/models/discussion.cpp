#include "../../include/models/discussion.h"
#include "../../include/database/database.h"
#include <json/json.h>
#include <iostream>
#include <vector>
#include <ctime>

// 讨论类的实现

// 构造函数
Discussion::Discussion() : id(0), problem_id(0), user_id(0), views(0), likes(0), created_at(0), updated_at(0) {}

Discussion::Discussion(int user_id, const std::string& title, const std::string& content, int problem_id)
    : id(0), problem_id(problem_id), user_id(user_id), title(title), content(content), 
      views(0), likes(0), created_at(std::time(nullptr)), updated_at(std::time(nullptr)) {}

// Getters
int Discussion::getId() const { return id; }
int Discussion::getProblemId() const { return problem_id; }
int Discussion::getUserId() const { return user_id; }
std::string Discussion::getTitle() const { return title; }
std::string Discussion::getContent() const { return content; }
int Discussion::getViews() const { return views; }
int Discussion::getLikes() const { return likes; }
std::time_t Discussion::getCreatedAt() const { return created_at; }
std::time_t Discussion::getUpdatedAt() const { return updated_at; }

// Setters
void Discussion::setId(int id) { this->id = id; }
void Discussion::setProblemId(int problem_id) { this->problem_id = problem_id; }
void Discussion::setUserId(int user_id) { this->user_id = user_id; }
void Discussion::setTitle(const std::string& title) { this->title = title; }
void Discussion::setContent(const std::string& content) { this->content = content; }
void Discussion::setViews(int views) { this->views = views; }
void Discussion::setLikes(int likes) { this->likes = likes; }
void Discussion::setCreatedAt(std::time_t created_at) { this->created_at = created_at; }
void Discussion::setUpdatedAt(std::time_t updated_at) { this->updated_at = updated_at; }

// 增加浏览量
void Discussion::incrementViews() { this->views++; }

// 增加/减少点赞数
void Discussion::incrementLikes() { this->likes++; }
void Discussion::decrementLikes() { this->likes--; }

// 从JSON字符串加载讨论
Discussion Discussion::fromJson(const std::string& json_str) {
    Discussion discussion;
    Json::Value root;
    Json::Reader reader;
    
    if (reader.parse(json_str, root)) {
        if (!root["title"].isNull()) discussion.setTitle(root["title"].asString());
        if (!root["content"].isNull()) discussion.setContent(root["content"].asString());
        if (!root["problem_id"].isNull()) discussion.setProblemId(root["problem_id"].asInt());
        if (!root["user_id"].isNull()) discussion.setUserId(root["user_id"].asInt());
    }
    
    return discussion;
}

// 转换为JSON字符串
std::string Discussion::toJson() const {
    Json::Value root;
    root["id"] = id;
    root["problem_id"] = problem_id;
    root["user_id"] = user_id;
    root["title"] = title;
    root["content"] = content;
    root["views"] = views;
    root["likes"] = likes;
    root["created_at"] = static_cast<Json::Int64>(created_at);
    root["updated_at"] = static_cast<Json::Int64>(updated_at);
    
    Json::FastWriter writer;
    return writer.write(root);
}

// 讨论回复类的实现

// 构造函数
DiscussionReply::DiscussionReply() : id(0), discussion_id(0), user_id(0), parent_id(0), likes(0), created_at(0), updated_at(0) {}

DiscussionReply::DiscussionReply(int discussion_id, int user_id, const std::string& content, int parent_id)
    : id(0), discussion_id(discussion_id), user_id(user_id), parent_id(parent_id), content(content),
      likes(0), created_at(std::time(nullptr)), updated_at(std::time(nullptr)) {}

// Getters
int DiscussionReply::getId() const { return id; }
int DiscussionReply::getDiscussionId() const { return discussion_id; }
int DiscussionReply::getUserId() const { return user_id; }
int DiscussionReply::getParentId() const { return parent_id; }
std::string DiscussionReply::getContent() const { return content; }
int DiscussionReply::getLikes() const { return likes; }
std::time_t DiscussionReply::getCreatedAt() const { return created_at; }
std::time_t DiscussionReply::getUpdatedAt() const { return updated_at; }

// Setters
void DiscussionReply::setId(int id) { this->id = id; }
void DiscussionReply::setDiscussionId(int discussion_id) { this->discussion_id = discussion_id; }
void DiscussionReply::setUserId(int user_id) { this->user_id = user_id; }
void DiscussionReply::setParentId(int parent_id) { this->parent_id = parent_id; }
void DiscussionReply::setContent(const std::string& content) { this->content = content; }
void DiscussionReply::setLikes(int likes) { this->likes = likes; }
void DiscussionReply::setCreatedAt(std::time_t created_at) { this->created_at = created_at; }
void DiscussionReply::setUpdatedAt(std::time_t updated_at) { this->updated_at = updated_at; }

// 增加/减少点赞数
void DiscussionReply::incrementLikes() { this->likes++; }
void DiscussionReply::decrementLikes() { this->likes--; }

// 从JSON字符串加载回复
DiscussionReply DiscussionReply::fromJson(const std::string& json_str) {
    DiscussionReply reply;
    Json::Value root;
    Json::Reader reader;
    
    if (reader.parse(json_str, root)) {
        if (!root["discussion_id"].isNull()) reply.setDiscussionId(root["discussion_id"].asInt());
        if (!root["user_id"].isNull()) reply.setUserId(root["user_id"].asInt());
        if (!root["parent_id"].isNull()) reply.setParentId(root["parent_id"].asInt());
        if (!root["content"].isNull()) reply.setContent(root["content"].asString());
    }
    
    return reply;
}

// 转换为JSON字符串
std::string DiscussionReply::toJson() const {
    Json::Value root;
    root["id"] = id;
    root["discussion_id"] = discussion_id;
    root["user_id"] = user_id;
    root["parent_id"] = parent_id;
    root["content"] = content;
    root["likes"] = likes;
    root["created_at"] = static_cast<Json::Int64>(created_at);
    root["updated_at"] = static_cast<Json::Int64>(updated_at);
    
    Json::FastWriter writer;
    return writer.write(root);
}

// 讨论数据访问对象的实现
bool DiscussionDAO::createDiscussion(Discussion& discussion) {
    try {
        // 获取数据库连接
        auto conn = Database::getInstance()->getConnection();
        
        // 准备SQL语句
        std::string sql = "INSERT INTO discussions (problem_id, user_id, title, content, views, likes, created_at, updated_at) "
                        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
        
        // 使用mysql_query直接执行SQL
        // 这里简化实现，实际应使用参数化查询避免SQL注入
        std::string insert_sql = "INSERT INTO discussions (problem_id, user_id, title, content, views, likes, created_at, updated_at) "
                               "VALUES (" + std::to_string(discussion.getProblemId()) + ", " 
                               + std::to_string(discussion.getUserId()) + ", '"
                               + Database::getInstance()->escapeString(discussion.getTitle()) + "', '"
                               + Database::getInstance()->escapeString(discussion.getContent()) + "', "
                               + std::to_string(discussion.getViews()) + ", "
                               + std::to_string(discussion.getLikes()) + ", "
                               + std::to_string(discussion.getCreatedAt()) + ", "
                               + std::to_string(discussion.getUpdatedAt()) + ")";
        
        if (mysql_query(conn, insert_sql.c_str()) != 0) {
            std::cerr << "插入讨论失败: " << mysql_error(conn) << std::endl;
            return false;
        }
        
        // 获取自增ID
        discussion.setId(static_cast<int>(mysql_insert_id(conn)));
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "创建讨论失败: " << e.what() << std::endl;
        return false;
    }
}

Discussion DiscussionDAO::getDiscussionById(int id) {
    Discussion discussion;
    
    try {
        // 获取数据库连接
        auto conn = Database::getInstance()->getConnection();
        
        // 构建SQL语句
        std::string sql = "SELECT id, problem_id, user_id, title, content, views, likes, created_at, updated_at "
                        "FROM discussions WHERE id = " + std::to_string(id);
        
        // 执行查询
        if (mysql_query(conn, sql.c_str()) != 0) {
            std::cerr << "查询讨论失败: " << mysql_error(conn) << std::endl;
            return discussion;
        }
        
        // 获取结果集
        MYSQL_RES* result = mysql_store_result(conn);
        if (result == nullptr) {
            std::cerr << "获取结果集失败: " << mysql_error(conn) << std::endl;
            return discussion;
        }
        
        // 处理结果
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row) {
            discussion.setId(std::stoi(row[0]));
            discussion.setProblemId(std::stoi(row[1]));
            discussion.setUserId(std::stoi(row[2]));
            discussion.setTitle(row[3]);
            discussion.setContent(row[4]);
            discussion.setViews(std::stoi(row[5]));
            discussion.setLikes(std::stoi(row[6]));
            discussion.setCreatedAt(std::stoll(row[7]));
            discussion.setUpdatedAt(std::stoll(row[8]));
        }
        
        // 释放结果集
        mysql_free_result(result);
    } catch (const std::exception& e) {
        std::cerr << "获取讨论失败: " << e.what() << std::endl;
    }
    
    return discussion;
}

bool DiscussionDAO::updateDiscussion(const Discussion& discussion) {
    try {
        // 获取数据库连接
        auto conn = Database::getInstance()->getConnection();
        
        // 构建SQL语句
        std::string sql = "UPDATE discussions SET problem_id = " + std::to_string(discussion.getProblemId()) + 
                          ", user_id = " + std::to_string(discussion.getUserId()) + 
                          ", title = '" + Database::getInstance()->escapeString(discussion.getTitle()) + "'" +
                          ", content = '" + Database::getInstance()->escapeString(discussion.getContent()) + "'" +
                          ", views = " + std::to_string(discussion.getViews()) + 
                          ", likes = " + std::to_string(discussion.getLikes()) + 
                          ", updated_at = " + std::to_string(std::time(nullptr)) + 
                          " WHERE id = " + std::to_string(discussion.getId());
        
        // 执行更新
        if (mysql_query(conn, sql.c_str()) != 0) {
            std::cerr << "更新讨论失败: " << mysql_error(conn) << std::endl;
            return false;
        }
        
        // 检查影响的行数
        return mysql_affected_rows(conn) > 0;
    } catch (const std::exception& e) {
        std::cerr << "更新讨论失败: " << e.what() << std::endl;
        return false;
    }
}

bool DiscussionDAO::deleteDiscussion(int id) {
    try {
        // 获取数据库连接
        auto conn = Database::getInstance()->getConnection();
        
        // 构建SQL语句
        std::string sql = "DELETE FROM discussions WHERE id = " + std::to_string(id);
        
        // 执行删除
        if (mysql_query(conn, sql.c_str()) != 0) {
            std::cerr << "删除讨论失败: " << mysql_error(conn) << std::endl;
            return false;
        }
        
        // 检查影响的行数
        return mysql_affected_rows(conn) > 0;
    } catch (const std::exception& e) {
        std::cerr << "删除讨论失败: " << e.what() << std::endl;
        return false;
    }
}

std::vector<Discussion> DiscussionDAO::getAllDiscussions(int offset, int limit) {
    std::vector<Discussion> discussions;
    
    try {
        // 获取数据库连接
        auto conn = Database::getInstance()->getConnection();
        
        // 构建SQL语句
        std::string sql = "SELECT id, problem_id, user_id, title, content, views, likes, created_at, updated_at "
                        "FROM discussions ORDER BY created_at DESC LIMIT " + std::to_string(limit) + 
                        " OFFSET " + std::to_string(offset);
        
        // 执行查询
        if (mysql_query(conn, sql.c_str()) != 0) {
            std::cerr << "查询所有讨论失败: " << mysql_error(conn) << std::endl;
            return discussions;
        }
        
        // 获取结果集
        MYSQL_RES* result = mysql_store_result(conn);
        if (result == nullptr) {
            std::cerr << "获取结果集失败: " << mysql_error(conn) << std::endl;
            return discussions;
        }
        
        // 处理结果
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            Discussion discussion;
            discussion.setId(std::stoi(row[0]));
            discussion.setProblemId(std::stoi(row[1]));
            discussion.setUserId(std::stoi(row[2]));
            discussion.setTitle(row[3]);
            discussion.setContent(row[4]);
            discussion.setViews(std::stoi(row[5]));
            discussion.setLikes(std::stoi(row[6]));
            discussion.setCreatedAt(std::stoll(row[7]));
            discussion.setUpdatedAt(std::stoll(row[8]));
            discussions.push_back(discussion);
        }
        
        // 释放结果集
        mysql_free_result(result);
    } catch (const std::exception& e) {
        std::cerr << "获取所有讨论失败: " << e.what() << std::endl;
    }
    
    return discussions;
}

std::vector<Discussion> DiscussionDAO::getDiscussionsByProblemId(int problem_id, int offset, int limit) {
    std::vector<Discussion> discussions;
    
    try {
        // 获取数据库连接
        auto conn = Database::getInstance()->getConnection();
        
        // 构建SQL语句
        std::string sql = "SELECT id, problem_id, user_id, title, content, views, likes, created_at, updated_at "
                        "FROM discussions WHERE problem_id = " + std::to_string(problem_id) + 
                        " ORDER BY created_at DESC LIMIT " + std::to_string(limit) + 
                        " OFFSET " + std::to_string(offset);
        
        // 执行查询
        if (mysql_query(conn, sql.c_str()) != 0) {
            std::cerr << "查询题目讨论失败: " << mysql_error(conn) << std::endl;
            return discussions;
        }
        
        // 获取结果集
        MYSQL_RES* result = mysql_store_result(conn);
        if (result == nullptr) {
            std::cerr << "获取结果集失败: " << mysql_error(conn) << std::endl;
            return discussions;
        }
        
        // 处理结果
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            Discussion discussion;
            discussion.setId(std::stoi(row[0]));
            discussion.setProblemId(std::stoi(row[1]));
            discussion.setUserId(std::stoi(row[2]));
            discussion.setTitle(row[3]);
            discussion.setContent(row[4]);
            discussion.setViews(std::stoi(row[5]));
            discussion.setLikes(std::stoi(row[6]));
            discussion.setCreatedAt(std::stoll(row[7]));
            discussion.setUpdatedAt(std::stoll(row[8]));
            discussions.push_back(discussion);
        }
        
        // 释放结果集
        mysql_free_result(result);
    } catch (const std::exception& e) {
        std::cerr << "获取题目讨论失败: " << e.what() << std::endl;
    }
    
    return discussions;
}

// 其他DAO方法实现省略，可按需添加后续实现...

// 讨论回复DAO实现
bool DiscussionDAO::createDiscussionReply(DiscussionReply& reply) {
    try {
        // 获取数据库连接
        auto conn = Database::getInstance()->getConnection();
        
        // 构建SQL语句
        std::string sql = "INSERT INTO discussion_replies (discussion_id, user_id, parent_id, content, likes, created_at, updated_at) "
                      "VALUES (" + std::to_string(reply.getDiscussionId()) + ", " 
                      + std::to_string(reply.getUserId()) + ", "
                      + std::to_string(reply.getParentId()) + ", '"
                      + Database::getInstance()->escapeString(reply.getContent()) + "', "
                      + std::to_string(reply.getLikes()) + ", "
                      + std::to_string(reply.getCreatedAt()) + ", "
                      + std::to_string(reply.getUpdatedAt()) + ")";
        
        // 执行插入
        if (mysql_query(conn, sql.c_str()) != 0) {
            std::cerr << "插入回复失败: " << mysql_error(conn) << std::endl;
            return false;
        }
        
        // 获取自增ID
        reply.setId(static_cast<int>(mysql_insert_id(conn)));
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "创建回复失败: " << e.what() << std::endl;
        return false;
    }
}

std::vector<DiscussionReply> DiscussionDAO::getRepliesByDiscussionId(int discussion_id, int offset, int limit) {
    std::vector<DiscussionReply> replies;
    
    try {
        // 获取数据库连接
        auto conn = Database::getInstance()->getConnection();
        
        // 构建SQL语句 - 获取顶层回复（parent_id=0）
        std::string sql = "SELECT id, discussion_id, user_id, parent_id, content, likes, created_at, updated_at "
                        "FROM discussion_replies WHERE discussion_id = " + std::to_string(discussion_id) + 
                        " AND parent_id = 0 ORDER BY created_at ASC LIMIT " + std::to_string(limit) + 
                        " OFFSET " + std::to_string(offset);
        
        // 执行查询
        if (mysql_query(conn, sql.c_str()) != 0) {
            std::cerr << "查询讨论回复失败: " << mysql_error(conn) << std::endl;
            return replies;
        }
        
        // 获取结果集
        MYSQL_RES* result = mysql_store_result(conn);
        if (result == nullptr) {
            std::cerr << "获取结果集失败: " << mysql_error(conn) << std::endl;
            return replies;
        }
        
        // 处理结果
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            DiscussionReply reply;
            reply.setId(std::stoi(row[0]));
            reply.setDiscussionId(std::stoi(row[1]));
            reply.setUserId(std::stoi(row[2]));
            reply.setParentId(std::stoi(row[3]));
            reply.setContent(row[4]);
            reply.setLikes(std::stoi(row[5]));
            reply.setCreatedAt(std::stoll(row[6]));
            reply.setUpdatedAt(std::stoll(row[7]));
            replies.push_back(reply);
        }
        
        // 释放结果集
        mysql_free_result(result);
    } catch (const std::exception& e) {
        std::cerr << "获取讨论回复失败: " << e.what() << std::endl;
    }
    
    return replies;
}

// 获取子回复
std::vector<DiscussionReply> DiscussionDAO::getChildReplies(int parent_id, int offset, int limit) {
    std::vector<DiscussionReply> replies;
    
    try {
        // 获取数据库连接
        auto conn = Database::getInstance()->getConnection();
        
        // 构建SQL语句
        std::string sql = "SELECT id, discussion_id, user_id, parent_id, content, likes, created_at, updated_at "
                        "FROM discussion_replies WHERE parent_id = " + std::to_string(parent_id) + 
                        " ORDER BY created_at ASC LIMIT " + std::to_string(limit) + 
                        " OFFSET " + std::to_string(offset);
        
        // 执行查询
        if (mysql_query(conn, sql.c_str()) != 0) {
            std::cerr << "查询子回复失败: " << mysql_error(conn) << std::endl;
            return replies;
        }
        
        // 获取结果集
        MYSQL_RES* result = mysql_store_result(conn);
        if (result == nullptr) {
            std::cerr << "获取结果集失败: " << mysql_error(conn) << std::endl;
            return replies;
        }
        
        // 处理结果
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            DiscussionReply reply;
            reply.setId(std::stoi(row[0]));
            reply.setDiscussionId(std::stoi(row[1]));
            reply.setUserId(std::stoi(row[2]));
            reply.setParentId(std::stoi(row[3]));
            reply.setContent(row[4]);
            reply.setLikes(std::stoi(row[5]));
            reply.setCreatedAt(std::stoll(row[6]));
            reply.setUpdatedAt(std::stoll(row[7]));
            replies.push_back(reply);
        }
        
        // 释放结果集
        mysql_free_result(result);
    } catch (const std::exception& e) {
        std::cerr << "获取子回复失败: " << e.what() << std::endl;
    }
    
    return replies;
}

// 获取回复详情
DiscussionReply DiscussionDAO::getDiscussionReplyById(int id) {
    DiscussionReply reply;
    
    try {
        // 获取数据库连接
        auto conn = Database::getInstance()->getConnection();
        
        // 构建SQL语句
        std::string sql = "SELECT id, discussion_id, user_id, parent_id, content, likes, created_at, updated_at "
                        "FROM discussion_replies WHERE id = " + std::to_string(id);
        
        // 执行查询
        if (mysql_query(conn, sql.c_str()) != 0) {
            std::cerr << "查询回复失败: " << mysql_error(conn) << std::endl;
            return reply;
        }
        
        // 获取结果集
        MYSQL_RES* result = mysql_store_result(conn);
        if (result == nullptr) {
            std::cerr << "获取结果集失败: " << mysql_error(conn) << std::endl;
            return reply;
        }
        
        // 处理结果
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row) {
            reply.setId(std::stoi(row[0]));
            reply.setDiscussionId(std::stoi(row[1]));
            reply.setUserId(std::stoi(row[2]));
            reply.setParentId(std::stoi(row[3]));
            reply.setContent(row[4]);
            reply.setLikes(std::stoi(row[5]));
            reply.setCreatedAt(std::stoll(row[6]));
            reply.setUpdatedAt(std::stoll(row[7]));
        }
        
        // 释放结果集
        mysql_free_result(result);
    } catch (const std::exception& e) {
        std::cerr << "获取回复失败: " << e.what() << std::endl;
    }
    
    return reply;
}

// 删除回复
bool DiscussionDAO::deleteDiscussionReply(int id) {
    try {
        // 获取数据库连接
        auto conn = Database::getInstance()->getConnection();
        
        // 构建SQL语句
        std::string sql = "DELETE FROM discussion_replies WHERE id = " + std::to_string(id);
        
        // 执行删除
        if (mysql_query(conn, sql.c_str()) != 0) {
            std::cerr << "删除回复失败: " << mysql_error(conn) << std::endl;
            return false;
        }
        
        // 检查影响的行数
        return mysql_affected_rows(conn) > 0;
    } catch (const std::exception& e) {
        std::cerr << "删除回复失败: " << e.what() << std::endl;
        return false;
    }
} 