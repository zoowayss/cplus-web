#ifndef PROBLEM_H
#define PROBLEM_H

#include <string>
#include <vector>
#include <ctime>

// 题目难度枚举
enum class ProblemDifficulty {
    EASY = 0,       // 简单
    MEDIUM = 1,     // 中等
    HARD = 2        // 困难
};

// 测试用例类
class TestCase {
private:
    int id;                  // 测试用例ID
    int problem_id;          // 关联的题目ID
    std::string input;       // 输入数据
    std::string output;      // 期望输出
    bool is_sample;          // 是否为样例
    int time_limit;          // 时间限制（毫秒）
    int memory_limit;        // 内存限制（KB）
    std::time_t created_at;  // 创建时间

public:
    // 构造函数
    TestCase();
    TestCase(int problem_id, const std::string& input, const std::string& output, 
             bool is_sample = false, int time_limit = 1000, int memory_limit = 65536);
    
    // Getters
    int getId() const;
    int getProblemId() const;
    std::string getInput() const;
    std::string getOutput() const;
    bool isSample() const;
    int getTimeLimit() const;
    int getMemoryLimit() const;
    std::time_t getCreatedAt() const;
    
    // Setters
    void setId(int id);
    void setProblemId(int problem_id);
    void setInput(const std::string& input);
    void setOutput(const std::string& output);
    void setIsSample(bool is_sample);
    void setTimeLimit(int time_limit);
    void setMemoryLimit(int memory_limit);
    void setCreatedAt(std::time_t created_at);
    
    // 从JSON字符串加载测试用例
    static TestCase fromJson(const std::string& json);
    
    // 转换为JSON字符串
    std::string toJson() const;
};

// 题目类
class Problem {
private:
    int id;                     // 题目ID
    std::string title;          // 标题
    std::string description;    // 描述
    std::string input_format;   // 输入格式
    std::string output_format;  // 输出格式
    std::string constraints;    // 约束条件
    std::string hint;           // 提示
    int time_limit;             // 时间限制（毫秒）
    int memory_limit;           // 内存限制（KB）
    ProblemDifficulty difficulty; // 难度
    bool is_public;             // 是否公开
    int created_by;             // 创建者ID
    std::time_t created_at;     // 创建时间
    std::time_t updated_at;     // 更新时间
    std::vector<TestCase> test_cases; // 测试用例

public:
    // 构造函数
    Problem();
    Problem(const std::string& title, const std::string& description);
    
    // Getters
    int getId() const;
    std::string getTitle() const;
    std::string getDescription() const;
    std::string getInputFormat() const;
    std::string getOutputFormat() const;
    std::string getConstraints() const;
    std::string getHint() const;
    int getTimeLimit() const;
    int getMemoryLimit() const;
    ProblemDifficulty getDifficulty() const;
    bool isPublic() const;
    int getCreatedBy() const;
    std::time_t getCreatedAt() const;
    std::time_t getUpdatedAt() const;
    std::vector<TestCase> getTestCases() const;
    std::vector<TestCase> getSampleTestCases() const;
    
    // Setters
    void setId(int id);
    void setTitle(const std::string& title);
    void setDescription(const std::string& description);
    void setInputFormat(const std::string& input_format);
    void setOutputFormat(const std::string& output_format);
    void setConstraints(const std::string& constraints);
    void setHint(const std::string& hint);
    void setTimeLimit(int time_limit);
    void setMemoryLimit(int memory_limit);
    void setDifficulty(ProblemDifficulty difficulty);
    void setIsPublic(bool is_public);
    void setCreatedBy(int created_by);
    void setCreatedAt(std::time_t created_at);
    void setUpdatedAt(std::time_t updated_at);
    void setTestCases(const std::vector<TestCase>& test_cases);
    
    // 添加测试用例
    void addTestCase(const TestCase& test_case);
    
    // 从JSON字符串加载题目
    static Problem fromJson(const std::string& json);
    
    // 转换为JSON字符串
    std::string toJson() const;
};

// 题目数据访问对象，处理题目相关的数据库操作
class ProblemDAO {
public:
    // 创建题目
    static bool createProblem(Problem& problem);
    
    // 通过ID获取题目
    static Problem getProblemById(int id, bool include_test_cases = false);
    
    // 更新题目信息
    static bool updateProblem(const Problem& problem);
    
    // 删除题目
    static bool deleteProblem(int id);
    
    // 获取所有题目
    static std::vector<Problem> getAllProblems(int offset = 0, int limit = 10, bool include_test_cases = false);
    
    // 获取题目总数
    static int getProblemCount();
    
    // 通过用户ID获取创建的题目
    static std::vector<Problem> getProblemsByCreator(int creator_id, int offset = 0, int limit = 10);
    
    // 通过难度获取题目
    static std::vector<Problem> getProblemsByDifficulty(ProblemDifficulty difficulty, int offset = 0, int limit = 10);
    
    // 添加测试用例
    static bool addTestCase(int problem_id, TestCase& test_case);
    
    // 更新测试用例
    static bool updateTestCase(const TestCase& test_case);
    
    // 删除测试用例
    static bool deleteTestCase(int test_case_id);
    
    // 获取题目的所有测试用例
    static std::vector<TestCase> getTestCasesByProblemId(int problem_id);
    
    // 获取题目的样例测试用例
    static std::vector<TestCase> getSampleTestCasesByProblemId(int problem_id);
};

#endif // PROBLEM_H 