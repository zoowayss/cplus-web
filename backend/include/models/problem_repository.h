#ifndef PROBLEM_REPOSITORY_H
#define PROBLEM_REPOSITORY_H

#include <vector>
#include <string>
#include "problem.h"

// 问题仓库类，负责问题相关的数据库操作
class ProblemRepository {
public:
    // 创建题目
    static bool createProblem(Problem& problem);
    
    // 根据ID获取题目信息
    static Problem getProblemById(int problem_id, bool include_test_cases = false);
    
    // 更新题目
    static bool updateProblem(const Problem& problem);
    
    // 删除题目
    static bool deleteProblem(int problem_id);
    
    // 获取题目列表
    static std::vector<Problem> getProblems(int offset = 0, int limit = 10, const std::string& search = "");
    
    // 获取题目总数
    static int countProblems(const std::string& search = "");
    
    // 添加测试用例
    static bool addTestCase(int problem_id, TestCase& testcase);
    
    // 更新测试用例
    static bool updateTestCase(const TestCase& testcase);
    
    // 删除测试用例
    static bool deleteTestCase(int testcase_id);
    
    // 获取题目的测试用例
    static std::vector<TestCase> getTestCasesByProblemId(int problem_id);
    
    // 检查题目权限
    static bool checkProblemPermission(int problem_id, int user_id, int user_role);
};

#endif // PROBLEM_REPOSITORY_H 