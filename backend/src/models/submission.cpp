#include "../../include/models/submission.h"
#include <sstream>
#include <iostream>

// 测试点结果类构造函数
TestPointResult::TestPointResult() : id(0), submission_id(0), test_case_id(0), result(JudgeResult::PENDING), time_used(0), memory_used(0) {}

TestPointResult::TestPointResult(int submission_id, int test_case_id) 
    : id(0), submission_id(submission_id), test_case_id(test_case_id), result(JudgeResult::PENDING), time_used(0), memory_used(0) {}

// Getters
int TestPointResult::getId() const { return id; }
int TestPointResult::getSubmissionId() const { return submission_id; }
int TestPointResult::getTestCaseId() const { return test_case_id; }
JudgeResult TestPointResult::getResult() const { return result; }
int TestPointResult::getTimeUsed() const { return time_used; }
int TestPointResult::getMemoryUsed() const { return memory_used; }
std::string TestPointResult::getOutput() const { return output; }
std::string TestPointResult::getErrorMessage() const { return error_message; }

// Setters
void TestPointResult::setId(int id) { this->id = id; }
void TestPointResult::setSubmissionId(int submission_id) { this->submission_id = submission_id; }
void TestPointResult::setTestCaseId(int test_case_id) { this->test_case_id = test_case_id; }
void TestPointResult::setResult(JudgeResult result) { this->result = result; }
void TestPointResult::setTimeUsed(int time_used) { this->time_used = time_used; }
void TestPointResult::setMemoryUsed(int memory_used) { this->memory_used = memory_used; }
void TestPointResult::setOutput(const std::string& output) { this->output = output; }
void TestPointResult::setErrorMessage(const std::string& error_message) { this->error_message = error_message; }

// 从JSON字符串加载测试点结果
TestPointResult TestPointResult::fromJson(const std::string& json) {
    // TODO: 实现JSON解析
    TestPointResult result;
    return result;
}

// 转换为JSON对象
Json::Value TestPointResult::toJson() const {
    Json::Value root;
    root["id"] = id;
    root["submission_id"] = submission_id;
    root["test_case_id"] = test_case_id;
    root["result"] = static_cast<int>(result);
    root["time_used"] = time_used;
    root["memory_used"] = memory_used;
    root["output"] = output;
    root["error_message"] = error_message;
    return root;
}

// 提交记录类构造函数
Submission::Submission() 
    : id(0), user_id(0), problem_id(0), language(Language::CPP), result(JudgeResult::PENDING), 
      score(0), time_used(0), memory_used(0), created_at(0), judged_at(0) {}

Submission::Submission(int user_id, int problem_id, const std::string& source_code, Language language)
    : id(0), user_id(user_id), problem_id(problem_id), source_code(source_code), language(language),
      result(JudgeResult::PENDING), score(0), time_used(0), memory_used(0), 
      created_at(std::time(nullptr)), judged_at(0) {}

// Getters
int Submission::getId() const { return id; }
int Submission::getUserId() const { return user_id; }
int Submission::getProblemId() const { return problem_id; }
std::string Submission::getSourceCode() const { return source_code; }
Language Submission::getLanguage() const { return language; }
JudgeResult Submission::getResult() const { return result; }
int Submission::getScore() const { return score; }
int Submission::getTimeUsed() const { return time_used; }
int Submission::getMemoryUsed() const { return memory_used; }
std::string Submission::getErrorMessage() const { return error_message; }
std::time_t Submission::getCreatedAt() const { return created_at; }
std::time_t Submission::getJudgedAt() const { return judged_at; }
std::vector<TestPointResult> Submission::getTestResults() const { return test_results; }

// 将Language枚举转换为字符串
std::string Submission::getLanguageStr() const {
    switch (language) {
        case Language::C:
            return "c";
        case Language::CPP:
            return "cpp";
        case Language::JAVA:
            return "java";
        case Language::PYTHON:
            return "python";
        case Language::JAVASCRIPT:
            return "javascript";
        default:
            return "cpp";
    }
}

// 将字符串转换为Language枚举
Language Submission::convertStringToLanguage(const std::string& langStr) {
    if (langStr == "c") {
        return Language::C;
    } else if (langStr == "cpp") {
        return Language::CPP;
    } else if (langStr == "java") {
        return Language::JAVA;
    } else if (langStr == "python") {
        return Language::PYTHON;
    } else if (langStr == "javascript") {
        return Language::JAVASCRIPT;
    } else {
        return Language::CPP; // 默认为C++
    }
}

// Setters
void Submission::setId(int id) { this->id = id; }
void Submission::setUserId(int user_id) { this->user_id = user_id; }
void Submission::setProblemId(int problem_id) { this->problem_id = problem_id; }
void Submission::setSourceCode(const std::string& source_code) { this->source_code = source_code; }
void Submission::setLanguage(Language language) { this->language = language; }
void Submission::setResult(JudgeResult result) { this->result = result; }
void Submission::setScore(int score) { this->score = score; }
void Submission::setTimeUsed(int time_used) { this->time_used = time_used; }
void Submission::setMemoryUsed(int memory_used) { this->memory_used = memory_used; }
void Submission::setErrorMessage(const std::string& error_message) { this->error_message = error_message; }
void Submission::setCreatedAt(std::time_t created_at) { this->created_at = created_at; }
void Submission::setJudgedAt(std::time_t judged_at) { this->judged_at = judged_at; }
void Submission::setTestResults(const std::vector<TestPointResult>& test_results) { this->test_results = test_results; }

// 添加测试点结果
void Submission::addTestResult(const TestPointResult& test_result) {
    test_results.push_back(test_result);
}

// 计算最终评测结果
void Submission::calculateFinalResult() {
    if (test_results.empty()) {
        return;
    }
    
    // 初始化为通过状态
    result = JudgeResult::ACCEPTED;
    score = 100;
    time_used = 0;
    memory_used = 0;
    
    // 如果有任何一个测试点未通过，则整体未通过
    for (const auto& test_result : test_results) {
        // 更新最大时间和内存使用
        if (test_result.getTimeUsed() > time_used) {
            time_used = test_result.getTimeUsed();
        }
        if (test_result.getMemoryUsed() > memory_used) {
            memory_used = test_result.getMemoryUsed();
        }
        
        // 如果测试点结果不是通过状态，则整体状态为该测试点的状态
        if (test_result.getResult() != JudgeResult::ACCEPTED) {
            result = test_result.getResult();
            // 可以根据需要计算得分
            score = 0;
            break;
        }
    }
}

// 从JSON字符串加载提交记录
Submission Submission::fromJson(const std::string& json) {
    // TODO: 实现JSON解析
    Submission submission;
    return submission;
}

// 转换为JSON对象
Json::Value Submission::toJson(bool complete) const {
    Json::Value root;
    root["id"] = id;
    root["user_id"] = user_id;
    root["problem_id"] = problem_id;
    root["language"] = getLanguageStr();
    root["result"] = static_cast<int>(result);
    root["score"] = score;
    root["time_used"] = time_used;
    root["memory_used"] = memory_used;
    root["error_message"] = error_message;
    root["created_at"] = static_cast<Json::Int64>(created_at);
    
    // 如果是完整模式，添加源代码
    if (complete && !source_code.empty()) {
        root["source_code"] = source_code;
    }
    
    // 如果有username和problem_title，也包含它们
    if (!username.empty()) {
        root["username"] = username;
    }
    if (!problem_title.empty()) {
        root["problem_title"] = problem_title;
    }
    
    if (judged_at > 0) {
        root["judged_at"] = static_cast<Json::Int64>(judged_at);
    } else {
        root["judged_at"] = Json::Value::null;
    }
    
    return root;
} 