#ifndef PROBLEM_SERVICE_H
#define PROBLEM_SERVICE_H

#include <string>
#include <vector>
#include "../models/problem.h"

class ProblemService {
public:
    // 创建题目
    static bool createProblem(Problem& problem, int creator_id, std::string& error_message);
    
    // 获取题目信息
    static Problem getProblemInfo(int problem_id, bool include_test_cases = false);
    
    // 更新题目信息
    static bool updateProblem(const Problem& problem, int user_id, std::string& error_message);
    
    // 删除题目
    static bool deleteProblem(int problem_id, int user_id, std::string& error_message);
    
    // 获取题目列表
    static std::vector<Problem> getProblemList(int offset = 0, int limit = 10, 
                                             ProblemDifficulty difficulty = ProblemDifficulty::EASY, 
                                             bool include_test_cases = false);
    
    // 通过用户ID获取创建的题目
    static std::vector<Problem> getProblemsByCreator(int creator_id, int offset = 0, int limit = 10);
    
    // 获取题目的测试用例
    static std::vector<TestCase> getTestCases(int problem_id, bool only_sample = false);
    
    // 添加测试用例
    static bool addTestCase(TestCase& test_case, int user_id, std::string& error_message);
    
    // 更新测试用例
    static bool updateTestCase(const TestCase& test_case, int user_id, std::string& error_message);
    
    // 删除测试用例
    static bool deleteTestCase(int test_case_id, int user_id, std::string& error_message);
    
    // 检查用户是否有权限操作题目
    static bool checkProblemPermission(int problem_id, int user_id);

private:
    // 验证题目数据
    static bool validateProblem(const Problem& problem, std::string& error_message);
    
    // 验证测试用例数据
    static bool validateTestCase(const TestCase& test_case, std::string& error_message);
};

#endif // PROBLEM_SERVICE_H 