#ifndef RANKING_SERVICE_H
#define RANKING_SERVICE_H

#include <string>
#include <vector>
#include <map>
#include "../models/user.h"

// 用户排名项结构
struct UserRankingItem {
    int user_id;                   // 用户ID
    std::string username;          // 用户名
    std::string avatar;            // 头像
    int solved_count;              // 解题数量
    int total_submissions;         // 总提交数
    float acceptance_rate;         // 通过率
    int score;                     // 总分
    std::time_t last_submission;   // 最后提交时间
    
    // 构造函数
    UserRankingItem() : user_id(0), solved_count(0), total_submissions(0), 
                        acceptance_rate(0.0f), score(0), last_submission(0) {}
};

class RankingService {
public:
    // 获取用户排名列表
    static std::vector<UserRankingItem> getUserRankingList(int offset = 0, int limit = 10);
    
    // 获取用户排名信息
    static UserRankingItem getUserRanking(int user_id);
    
    // 获取用户排名位置
    static int getUserRankingPosition(int user_id);
    
    // 更新用户排名数据
    static bool updateUserRanking(int user_id);
    
    // 刷新所有用户排名数据
    static bool refreshAllRankings();
    
    // 获取排名统计信息
    static std::map<std::string, int> getRankingStats();
    
private:
    // 计算用户分数
    static int calculateUserScore(int user_id);
    
    // 更新排名表
    static bool updateRankingTable(int user_id, int solved_count, int total_submissions, 
                                 float acceptance_rate, int score, std::time_t last_submission);
};

#endif // RANKING_SERVICE_H 