#ifndef SUBMISSION_REPOSITORY_H
#define SUBMISSION_REPOSITORY_H

#include <vector>
#include <string>
#include "submission.h"

class SubmissionRepository {
public:
    // 创建提交记录
    static bool createSubmission(Submission& submission);
    
    // 通过ID获取提交记录
    static Submission getSubmissionById(int id, bool include_test_results = false);
    
    // 更新提交记录信息
    static bool updateSubmission(const Submission& submission);
    
    // 添加测试点结果
    static bool addTestResult(int submission_id, TestPointResult& test_result);
    
    // 获取用户的所有提交记录
    static std::vector<Submission> getSubmissionsByUserId(int user_id, int offset = 0, int limit = 10);
    
    // 获取题目的所有提交记录
    static std::vector<Submission> getSubmissionsByProblemId(int problem_id, int offset = 0, int limit = 10);
    
    // 获取用户在特定题目的所有提交记录
    static std::vector<Submission> getSubmissionsByUserAndProblem(int user_id, int problem_id, int offset = 0, int limit = 10);
    
    // 获取所有提交记录
    static std::vector<Submission> getAllSubmissions(int offset = 0, int limit = 10);
    
    // 获取提交记录总数
    static int getSubmissionCount();
    
    // 获取用户的提交记录总数
    static int getSubmissionCountByUserId(int user_id);
    
    // 获取题目的提交记录总数
    static int getSubmissionCountByProblemId(int problem_id);
    
    // 获取用户特定题目的提交记录总数
    static int getSubmissionCountByUserAndProblemId(int user_id, int problem_id);
    
    // 获取用户的通过题目数量
    static int getAcceptedProblemCountByUserId(int user_id);
};

#endif // SUBMISSION_REPOSITORY_H 