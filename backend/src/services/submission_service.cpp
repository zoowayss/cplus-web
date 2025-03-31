#include "../../include/services/submission_service.h"
#include "../../include/models/submission.h"
#include "../../include/models/submission_repository.h"
#include "../../include/services/judge_engine.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <unistd.h>

// 线程管理接口实现
void SubmissionService::ensureJudgeThreadRunning() {
    // 不再需要启动评测线程
    std::cout << "同步评测模式，无需启动评测线程" << std::endl;
}

void SubmissionService::stopJudgeThread() {
    // 不再需要停止评测线程
    std::cout << "同步评测模式，无需停止评测线程" << std::endl;
}

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

// 创建提交并直接评测
bool SubmissionService::createSubmission(Submission& submission, std::string& error_message) {
    // 验证提交数据
    if (!validateSubmission(submission, error_message)) {
        std::cerr << "提交验证失败: " << error_message << std::endl;
        return false;
    }
    
    // 使用仓库类保存到数据库
    bool success = SubmissionRepository::createSubmission(submission);
    
    if (success) {
        int submission_id = submission.getId();
        
        // 验证提交ID是否有效
        if (submission_id <= 0) {
            std::cerr << "错误: 创建提交记录成功但获取到无效的提交ID: " << submission_id << std::endl;
            error_message = "获取提交ID失败";
            return false;
        }
        
        std::cout << "创建提交成功，ID: " << submission_id << "，开始评测..." << std::endl;
        
        // 再次验证提交记录是否存在
        Submission check = SubmissionRepository::getSubmissionById(submission_id, false);
        if (check.getId() <= 0) {
            std::cerr << "错误: 创建的提交记录无法从数据库中检索到，ID: " << submission_id << std::endl;
            error_message = "提交记录创建后无法检索";
            return false;
        }
        
        std::cout << "提交记录验证成功，开始同步评测，用户ID: " 
                  << check.getUserId() << ", 题目ID: " << check.getProblemId() 
                  << ", 语言: " << check.getLanguageStr() << std::endl;
        
        // 更新提交状态为评测中
        check.setResult(JudgeResult::JUDGING);
        SubmissionRepository::updateSubmission(check);
        
        // 直接评测
        JudgeEngine judge_engine;
        std::string judge_error_message;
        
        try {
            std::cout << "【评测引擎】开始同步评测提交ID: " << submission_id << std::endl;
            bool judge_success = judge_engine.judge(submission_id, judge_error_message);
            
            if (judge_success) {
                std::cout << "【评测引擎】成功完成评测，提交ID: " << submission_id << std::endl;
            } else {
                std::cerr << "【评测引擎】评测失败，提交ID: " << submission_id << ", 错误: " << judge_error_message << std::endl;
                // 更新提交状态为系统错误
                judgeSubmission(submission_id, JudgeResult::SYSTEM_ERROR, 0, 0, 0, "评测失败: " + judge_error_message);
            }
        } catch (const std::exception& e) {
            std::cerr << "【评测引擎】评测异常，提交ID: " << submission_id << ", 错误: " << e.what() << std::endl;
            // 更新提交状态为系统错误
            judgeSubmission(submission_id, JudgeResult::SYSTEM_ERROR, 0, 0, 0, std::string("评测系统异常: ") + e.what());
        } catch (...) {
            std::cerr << "【评测引擎】评测发生未知异常，提交ID: " << submission_id << std::endl;
            // 更新提交状态为系统错误
            judgeSubmission(submission_id, JudgeResult::SYSTEM_ERROR, 0, 0, 0, "评测系统发生未知异常");
        }
    } else {
        std::cerr << "创建提交失败，数据库操作返回错误" << std::endl;
        error_message = "数据库操作失败";
    }
    
    return success;
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

// 准备用于评测的提交 - 现在直接进行评测
bool SubmissionService::prepareSubmissionForJudge(int submission_id) {
    // 获取提交记录
    Submission submission = SubmissionRepository::getSubmissionById(submission_id, false);
    if (submission.getId() <= 0) {
        return false;
    }
    
    // 更新状态为"评测中"
    submission.setResult(JudgeResult::JUDGING);
    bool updateSuccess = SubmissionRepository::updateSubmission(submission);
    
    if (!updateSuccess) {
        std::cerr << "无法更新提交状态为'评测中'" << std::endl;
        return false;
    }
    
    // 直接评测
    JudgeEngine judge_engine;
    std::string error_message;
    
    try {
        std::cout << "【评测引擎】重新评测提交ID: " << submission_id << std::endl;
        return judge_engine.judge(submission_id, error_message);
    } catch (const std::exception& e) {
        std::cerr << "【评测引擎】评测异常，提交ID: " << submission_id << ", 错误: " << e.what() << std::endl;
        judgeSubmission(submission_id, JudgeResult::SYSTEM_ERROR, 0, 0, 0, std::string("评测系统异常: ") + e.what());
        return false;
    } catch (...) {
        std::cerr << "【评测引擎】评测发生未知异常，提交ID: " << submission_id << std::endl;
        judgeSubmission(submission_id, JudgeResult::SYSTEM_ERROR, 0, 0, 0, "评测系统发生未知异常");
        return false;
    }
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