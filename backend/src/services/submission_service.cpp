#include "../../include/services/submission_service.h"
#include "../../include/models/submission.h"
#include "../../include/models/submission_repository.h"
#include <iostream>

// 获取题目的提交列表
std::vector<Submission> SubmissionService::getProblemSubmissions(int problem_id, int offset, int limit) {
    return SubmissionRepository::getSubmissionsByProblemId(problem_id, offset, limit);
}

// 获取用户在特定题目的提交列表
std::vector<Submission> SubmissionService::getUserProblemSubmissions(int user_id, int problem_id, int offset, int limit) {
    return SubmissionRepository::getSubmissionsByUserAndProblem(user_id, problem_id, offset, limit);
}

// 获取题目的提交记录总数
int SubmissionService::getProblemSubmissionsCount(int problem_id, int user_id, bool is_admin) {
    if (is_admin) {
        // 管理员可以看到所有提交
        return SubmissionRepository::getSubmissionCountByProblemId(problem_id);
    } else {
        // 普通用户只能看到自己的提交
        return SubmissionRepository::getSubmissionCountByUserAndProblemId(user_id, problem_id);
    }
}

// 创建提交
bool SubmissionService::createSubmission(Submission& submission, std::string& error_message) {
    // 验证提交数据
    if (!validateSubmission(submission, error_message)) {
        return false;
    }
    
    // 使用仓库类保存到数据库
    return SubmissionRepository::createSubmission(submission);
}

// 获取提交信息
Submission SubmissionService::getSubmissionInfo(int submission_id, bool include_test_results) {
    return SubmissionRepository::getSubmissionById(submission_id, include_test_results);
}

// 获取用户的提交列表
std::vector<Submission> SubmissionService::getUserSubmissions(int user_id, int offset, int limit) {
    return SubmissionRepository::getSubmissionsByUserId(user_id, offset, limit);
}

// 获取所有提交列表
std::vector<Submission> SubmissionService::getAllSubmissions(int offset, int limit) {
    return SubmissionRepository::getAllSubmissions(offset, limit);
}

// 评测提交
bool SubmissionService::judgeSubmission(int submission_id, JudgeResult result, int score,
                                        int time_used, int memory_used, const std::string& error_message) {
    // 获取提交记录
    Submission submission = SubmissionRepository::getSubmissionById(submission_id, false);
    if (submission.getId() <= 0) {
        return false;
    }
    
    // 更新评测结果
    submission.setResult(result);
    submission.setScore(score);
    submission.setTimeUsed(time_used);
    submission.setMemoryUsed(memory_used);
    submission.setErrorMessage(error_message);
    
    // 保存到数据库
    return SubmissionRepository::updateSubmission(submission);
}

// 添加测试点结果
bool SubmissionService::addTestResult(TestPointResult& test_result) {
    return SubmissionRepository::addTestResult(test_result.getSubmissionId(), test_result);
}

// 检查用户是否有权限查看提交
bool SubmissionService::checkSubmissionPermission(int submission_id, int user_id) {
    // 获取提交信息
    Submission submission = SubmissionRepository::getSubmissionById(submission_id, false);
    
    // 如果提交不存在，无权限
    if (submission.getId() <= 0) {
        return false;
    }
    
    // 如果是提交者本人，有权限
    if (submission.getUserId() == user_id) {
        return true;
    }
    
    // TODO: 这里可以添加其他权限检查，例如管理员权限
    
    return false;
}

// 获取用户通过题目数量
int SubmissionService::getUserAcceptedCount(int user_id) {
    return SubmissionRepository::getAcceptedProblemCountByUserId(user_id);
}

// 获取用户的提交总数
int SubmissionService::getUserSubmissionCount(int user_id) {
    return SubmissionRepository::getSubmissionCountByUserId(user_id);
}

// 计算用户的通过率
float SubmissionService::getUserAcceptanceRate(int user_id) {
    int accepted = getUserAcceptedCount(user_id);
    int total = getUserSubmissionCount(user_id);
    
    if (total == 0) {
        return 0.0f;
    }
    
    return static_cast<float>(accepted) / total;
}

// 验证提交数据
bool SubmissionService::validateSubmission(const Submission& submission, std::string& error_message) {
    // 检查用户ID
    if (submission.getUserId() <= 0) {
        error_message = "无效的用户ID";
        return false;
    }
    
    // 检查题目ID
    if (submission.getProblemId() <= 0) {
        error_message = "无效的题目ID";
        return false;
    }
    
    // 检查源代码
    if (submission.getSourceCode().empty()) {
        error_message = "源代码不能为空";
        return false;
    }
    
    // 源代码长度限制
    if (submission.getSourceCode().length() > 65535) {
        error_message = "源代码超过长度限制";
        return false;
    }
    
    return true;
}

// 准备用于评测的提交
bool SubmissionService::prepareSubmissionForJudge(int submission_id) {
    // 获取提交记录
    Submission submission = SubmissionRepository::getSubmissionById(submission_id, false);
    if (submission.getId() <= 0) {
        return false;
    }
    
    // 更新状态为"评测中"
    submission.setResult(JudgeResult::JUDGING);
    return SubmissionRepository::updateSubmission(submission);
}

// 更新提交状态
bool SubmissionService::updateSubmissionStatus(int submission_id, JudgeResult result) {
    // 获取提交记录
    Submission submission = SubmissionRepository::getSubmissionById(submission_id, false);
    if (submission.getId() <= 0) {
        return false;
    }
    
    // 更新状态
    submission.setResult(result);
    return SubmissionRepository::updateSubmission(submission);
} 