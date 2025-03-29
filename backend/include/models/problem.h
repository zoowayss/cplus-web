#ifndef PROBLEM_H
#define PROBLEM_H

#include <string>
#include <vector>
#include <ctime>
#include <json/json.h>

// 题目难度枚举
enum class ProblemDifficulty {
    EASY = 0,       // 简单
    MEDIUM = 1,     // 中等
    HARD = 2        // 困难
};

// 测试用例结构
struct TestCase {
    int id;
    int problem_id;
    std::string input;
    std::string expected_output;
    bool is_example;
    int64_t created_at;
    
    Json::Value toJson() const {
        Json::Value json;
        json["id"] = id;
        json["problem_id"] = problem_id;
        json["input"] = input;
        json["expected_output"] = expected_output;
        json["is_example"] = is_example;
        json["created_at"] = Json::Int64(created_at);
        return json;
    }
};

// 题目类
class Problem {
private:
    int id;
    std::string title;
    std::string description;
    std::string input_format;
    std::string output_format;
    std::string difficulty;
    int time_limit;
    int memory_limit;
    std::string example_input;
    std::string example_output;
    std::string hint;
    int created_by;
    int64_t created_at;
    int64_t updated_at;
    int status;
    std::vector<TestCase> testcases;

public:
    // 构造函数
    Problem() : id(0), time_limit(1000), memory_limit(65536), created_by(0), created_at(0), updated_at(0), status(1) {}
    
    // 设置和获取属性
    int getId() const { return id; }
    void setId(int value) { id = value; }
    
    const std::string& getTitle() const { return title; }
    void setTitle(const std::string& value) { title = value; }
    
    const std::string& getDescription() const { return description; }
    void setDescription(const std::string& value) { description = value; }
    
    const std::string& getInputFormat() const { return input_format; }
    void setInputFormat(const std::string& value) { input_format = value; }
    
    const std::string& getOutputFormat() const { return output_format; }
    void setOutputFormat(const std::string& value) { output_format = value; }
    
    const std::string& getDifficulty() const { return difficulty; }
    void setDifficulty(const std::string& value) { difficulty = value; }
    
    int getTimeLimit() const { return time_limit; }
    void setTimeLimit(int value) { time_limit = value; }
    
    int getMemoryLimit() const { return memory_limit; }
    void setMemoryLimit(int value) { memory_limit = value; }
    
    const std::string& getExampleInput() const { return example_input; }
    void setExampleInput(const std::string& value) { example_input = value; }
    
    const std::string& getExampleOutput() const { return example_output; }
    void setExampleOutput(const std::string& value) { example_output = value; }
    
    const std::string& getHint() const { return hint; }
    void setHint(const std::string& value) { hint = value; }
    
    int getCreatedBy() const { return created_by; }
    void setCreatedBy(int value) { created_by = value; }
    
    int64_t getCreatedAt() const { return created_at; }
    void setCreatedAt(int64_t value) { created_at = value; }
    
    int64_t getUpdatedAt() const { return updated_at; }
    void setUpdatedAt(int64_t value) { updated_at = value; }
    
    int getStatus() const { return status; }
    void setStatus(int value) { status = value; }
    
    const std::vector<TestCase>& getTestCases() const { return testcases; }
    void setTestCases(const std::vector<TestCase>& value) { testcases = value; }
    void addTestCase(const TestCase& testcase) { testcases.push_back(testcase); }
    
    // 将题目转换为JSON
    Json::Value toJson() const {
        Json::Value json;
        json["id"] = id;
        json["title"] = title;
        json["description"] = description;
        json["input_format"] = input_format;
        json["output_format"] = output_format;
        json["difficulty"] = difficulty;
        json["time_limit"] = time_limit;
        json["memory_limit"] = memory_limit;
        json["example_input"] = example_input;
        json["example_output"] = example_output;
        json["hint"] = hint;
        json["created_by"] = created_by;
        json["created_at"] = Json::Int64(created_at);
        json["updated_at"] = Json::Int64(updated_at);
        json["status"] = status;
        
        // 添加测试用例（可选）
        Json::Value testcasesJson(Json::arrayValue);
        for (const auto& testcase : testcases) {
            testcasesJson.append(testcase.toJson());
        }
        json["testcases"] = testcasesJson;
        
        return json;
    }
    
    // 从JSON创建题目
    static Problem fromJson(const Json::Value& json) {
        Problem problem;
        
        if (json.isMember("id")) problem.setId(json["id"].asInt());
        if (json.isMember("title")) problem.setTitle(json["title"].asString());
        if (json.isMember("description")) problem.setDescription(json["description"].asString());
        if (json.isMember("input_format")) problem.setInputFormat(json["input_format"].asString());
        if (json.isMember("output_format")) problem.setOutputFormat(json["output_format"].asString());
        if (json.isMember("difficulty")) problem.setDifficulty(json["difficulty"].asString());
        if (json.isMember("time_limit")) problem.setTimeLimit(json["time_limit"].asInt());
        if (json.isMember("memory_limit")) problem.setMemoryLimit(json["memory_limit"].asInt());
        if (json.isMember("example_input")) problem.setExampleInput(json["example_input"].asString());
        if (json.isMember("example_output")) problem.setExampleOutput(json["example_output"].asString());
        if (json.isMember("hint")) problem.setHint(json["hint"].asString());
        if (json.isMember("created_by")) problem.setCreatedBy(json["created_by"].asInt());
        if (json.isMember("created_at")) problem.setCreatedAt(json["created_at"].asInt64());
        if (json.isMember("updated_at")) problem.setUpdatedAt(json["updated_at"].asInt64());
        if (json.isMember("status")) problem.setStatus(json["status"].asInt());
        
        // 解析测试用例（可选）
        if (json.isMember("testcases") && json["testcases"].isArray()) {
            for (const auto& testcaseJson : json["testcases"]) {
                TestCase testcase;
                testcase.id = testcaseJson["id"].asInt();
                testcase.problem_id = testcaseJson["problem_id"].asInt();
                testcase.input = testcaseJson["input"].asString();
                testcase.expected_output = testcaseJson["expected_output"].asString();
                testcase.is_example = testcaseJson["is_example"].asBool();
                testcase.created_at = testcaseJson["created_at"].asInt64();
                problem.addTestCase(testcase);
            }
        }
        
        return problem;
    }
    
    // 将题目转换为JSON字符串
    std::string toJsonString() const {
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "";
        return Json::writeString(builder, toJson());
    }
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