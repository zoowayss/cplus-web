#ifndef PROBLEM_SERVICE_H
#define PROBLEM_SERVICE_H

#include "../models/problem.h"
#include <string>
#include <vector>
#include <map>

class ProblemService {
public:
    // 获取所有题目（带分页）
    static std::vector<Problem> getAllProblems(int offset = 0, int limit = 10, const std::string& search = "");
    
    // 获取题目详情
    static Problem getProblemById(int problem_id, bool with_testcases = false);
    
    // 创建题目
    static bool createProblem(const Problem& problem, std::string& error_message);
    
    // 更新题目
    static bool updateProblem(const Problem& problem, std::string& error_message);
    
    // 删除题目
    static bool deleteProblem(int problem_id, std::string& error_message);
    
    // 添加测试用例
    static bool addTestCase(int problem_id, const TestCase& testcase, std::string& error_message);
    
    // 更新测试用例
    static bool updateTestCase(const TestCase& testcase, std::string& error_message);
    
    // 删除测试用例
    static bool deleteTestCase(int testcase_id, std::string& error_message);
    
    // 获取题目的所有测试用例
    static std::vector<TestCase> getTestCasesByProblemId(int problem_id);
    
    // 获取题目的示例测试用例
    static std::vector<TestCase> getExampleTestCasesByProblemId(int problem_id);
    
    // 获取题目计数
    static int countProblems(const std::string& search = "");
    
    // 检查用户是否有权限操作题目（创建者或管理员）
    static bool checkProblemPermission(int problem_id, int user_id, int user_role);
};

#endif // PROBLEM_SERVICE_H 