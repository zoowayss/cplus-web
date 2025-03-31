#ifndef SUBMISSION_SERVICE_H
#define SUBMISSION_SERVICE_H

#include <string>
#include <vector>
#include "../models/submission.h"

class SubmissionService {
public:
    // 创建提交
    static bool createSubmission(Submission& submission, std::string& error_message);
    
    // 获取提交信息
    static Submission getSubmissionInfo(int submission_id, bool include_test_results = false);
    
    // 获取用户的提交列表
    static std::vector<Submission> getUserSubmissions(int user_id, int offset = 0, int limit = 10);
    
    // 获取题目的提交列表
    static std::vector<Submission> getProblemSubmissions(int problem_id, int offset = 0, int limit = 10);
    
    // 获取用户在特定题目的提交列表
    static std::vector<Submission> getUserProblemSubmissions(int user_id, int problem_id, int offset = 0, int limit = 10);
    
    // 获取题目的提交记录总数（可以根据用户角色筛选）
    static int getProblemSubmissionsCount(int problem_id, int user_id, bool is_admin = false);
    
    // 获取所有提交列表（管理员用）
    static std::vector<Submission> getAllSubmissions(int offset = 0, int limit = 10);
    
    // 评测提交（由评测系统调用）
    static bool judgeSubmission(int submission_id, JudgeResult result, int score,
                              int time_used, int memory_used, const std::string& error_message);
    
    // 添加测试点结果
    static bool addTestResult(TestPointResult& test_result);
    
    // 检查用户是否有权限查看提交
    static bool checkSubmissionPermission(int submission_id, int user_id);
    
    // 获取用户通过题目数量
    static int getUserAcceptedCount(int user_id);
    
    // 获取用户的提交总数
    static int getUserSubmissionCount(int user_id);
    
    // 计算用户的通过率
    static float getUserAcceptanceRate(int user_id);
    
    // 更新提交状态
    static bool updateSubmissionStatus(int submission_id, JudgeResult result);
    
    // 评测线程管理
    static void ensureJudgeThreadRunning();
    static void stopJudgeThread();

private:
    // 验证提交数据
    static bool validateSubmission(const Submission& submission, std::string& error_message);
    
    // 准备用于评测的提交
    static bool prepareSubmissionForJudge(int submission_id);
};

#endif // SUBMISSION_SERVICE_H 