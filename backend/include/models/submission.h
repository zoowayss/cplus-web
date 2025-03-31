#ifndef SUBMISSION_H
#define SUBMISSION_H

#include <string>
#include <vector>
#include <ctime>
#include <json/json.h>

// 编程语言枚举
enum class Language {
    C = 0,
    CPP = 1,
    JAVA = 2,
    PYTHON = 3,
    JAVASCRIPT = 4
};

// 评测结果枚举
enum class JudgeResult {
    PENDING = 0,           // 等待评测
    JUDGING = 1,           // 评测中
    ACCEPTED = 2,          // 通过
    WRONG_ANSWER = 3,      // 答案错误
    TIME_LIMIT_EXCEEDED = 4,  // 超时
    MEMORY_LIMIT_EXCEEDED = 5, // 内存超限
    RUNTIME_ERROR = 6,     // 运行时错误
    COMPILE_ERROR = 7,     // 编译错误
    SYSTEM_ERROR = 8       // 系统错误
};

// 测试点结果类
class TestPointResult {
private:
    int id;                      // 测试点结果ID
    int submission_id;           // 提交ID
    int test_case_id;            // 测试用例ID
    JudgeResult result;          // 评测结果
    bool passed;                 // 是否通过
    int time_used;               // 耗时（毫秒）
    int memory_used;             // 内存使用（KB）
    std::string output;          // 实际输出
    std::string error_message;   // 错误信息

public:
    // 构造函数
    TestPointResult();
    TestPointResult(int submission_id, int test_case_id);
    
    // Getters
    int getId() const;
    int getSubmissionId() const;
    int getTestCaseId() const;
    JudgeResult getResult() const;
    bool getPassed() const;
    int getTimeUsed() const;
    int getMemoryUsed() const;
    std::string getOutput() const;
    std::string getErrorMessage() const;
    
    // Setters
    void setId(int id);
    void setSubmissionId(int submission_id);
    void setTestCaseId(int test_case_id);
    void setResult(JudgeResult result);
    void setPassed(bool passed);
    void setTimeUsed(int time_used);
    void setMemoryUsed(int memory_used);
    void setOutput(const std::string& output);
    void setErrorMessage(const std::string& error_message);
    
    // 从JSON字符串加载测试点结果
    static TestPointResult fromJson(const std::string& json);
    
    // 转换为JSON对象
    Json::Value toJson() const;
};

// 提交记录类
class Submission {
private:
    int id;                        // 提交ID
    int user_id;                   // 用户ID
    int problem_id;                // 题目ID
    std::string source_code;       // 源代码
    Language language;             // 编程语言
    JudgeResult result;            // 评测结果
    int score;                     // 得分
    int time_used;                 // 最大耗时（毫秒）
    int memory_used;               // 最大内存使用（KB）
    std::string error_message;     // 错误信息
    std::time_t created_at;        // 创建时间
    std::time_t judged_at;         // 评测时间
    std::vector<TestPointResult> test_results; // 测试点结果
    
    // 扩展字段（非数据库字段，仅用于显示）
    std::string username;          // 用户名
    std::string problem_title;     // 题目标题

public:
    // 构造函数
    Submission();
    Submission(int user_id, int problem_id, const std::string& source_code, Language language);
    
    // Getters
    int getId() const;
    int getUserId() const;
    int getProblemId() const;
    std::string getSourceCode() const;
    Language getLanguage() const;
    JudgeResult getResult() const;
    int getScore() const;
    int getTimeUsed() const;
    int getMemoryUsed() const;
    std::string getErrorMessage() const;
    std::time_t getCreatedAt() const;
    std::time_t getJudgedAt() const;
    std::vector<TestPointResult> getTestResults() const;
    
    // 扩展字段Getters
    std::string getUsername() const { return username; }
    std::string getProblemTitle() const { return problem_title; }
    
    // 将Language枚举转换为字符串
    std::string getLanguageStr() const;
    
    // 将字符串转换为Language枚举
    static Language convertStringToLanguage(const std::string& langStr);
    
    // Setters
    void setId(int id);
    void setUserId(int user_id);
    void setProblemId(int problem_id);
    void setSourceCode(const std::string& source_code);
    void setLanguage(Language language);
    
    // 添加一个接受字符串的setLanguage重载
    void setLanguage(const std::string& languageStr) { 
        this->language = convertStringToLanguage(languageStr); 
    }
    
    void setResult(JudgeResult result);
    void setScore(int score);
    void setTimeUsed(int time_used);
    void setMemoryUsed(int memory_used);
    void setErrorMessage(const std::string& error_message);
    void setCreatedAt(std::time_t created_at);
    void setJudgedAt(std::time_t judged_at);
    void setTestResults(const std::vector<TestPointResult>& test_results);
    
    // 扩展字段Setters
    void setUsername(const std::string& username) { this->username = username; }
    void setProblemTitle(const std::string& problem_title) { this->problem_title = problem_title; }
    
    // 添加测试点结果
    void addTestResult(const TestPointResult& test_result);
    
    // 计算最终评测结果
    void calculateFinalResult();
    
    // 从JSON字符串加载提交记录
    static Submission fromJson(const std::string& json);
    
    // 转换为JSON对象
    Json::Value toJson(bool complete = false) const;
};

// 提交记录数据访问对象，处理提交记录相关的数据库操作
class SubmissionDAO {
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
    
    // 获取用户的通过题目数量
    static int getAcceptedProblemCountByUserId(int user_id);
};

#endif // SUBMISSION_H 