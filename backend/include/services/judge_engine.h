#ifndef JUDGE_ENGINE_H
#define JUDGE_ENGINE_H

#include <string>
#include <vector>
#include <memory>
#include <iomanip>  // 为visualizeString添加
#include "../models/submission.h"
#include "../models/problem.h"

// 编译结果结构体
struct CompileResult {
    bool success;
    std::string error_message;
    std::string executable_path;
    
    CompileResult() : success(false) {}
    
    CompileResult(bool success, const std::string& error_message = "", const std::string& executable_path = "")
        : success(success), error_message(error_message), executable_path(executable_path) {}
};

// 运行结果结构体
struct ExecutionResult {
    bool success;
    JudgeResult result;
    std::string output;
    std::string error_message;
    int exit_code;
    int time_used_ms;
    int memory_used_kb;
    
    ExecutionResult() : success(false), result(JudgeResult::SYSTEM_ERROR), exit_code(-1), time_used_ms(0), memory_used_kb(0) {}
};

// 测试点结果
struct TestPointExecutionResult {
    int test_case_id;
    bool passed;
    JudgeResult result;
    std::string output;
    std::string expected;
    std::string error_message;
    int time_used_ms;
    int memory_used_kb;
    
    TestPointExecutionResult() : test_case_id(0), passed(false), result(JudgeResult::SYSTEM_ERROR), time_used_ms(0), memory_used_kb(0) {}
};

// 评测引擎类
class JudgeEngine {
public:
    // 构造函数
    JudgeEngine();
    
    // 析构函数
    ~JudgeEngine();
    
    // 对一个提交进行评测
    bool judge(int submission_id, std::string& error_message);
    
    // 设置工作目录
    void setWorkDir(const std::string& dir);
    
    // 设置超时限制（毫秒）
    void setTimeLimit(int ms);
    
    // 设置内存限制（KB）
    void setMemoryLimit(int kb);
    
private:
    // 编译代码
    CompileResult compileCode(const std::string& source_code, const std::string& language, const std::string& compile_dir);
    
    // 执行程序
    ExecutionResult executeProgram(const std::string& executable_path, const std::string& input, int time_limit_ms, int memory_limit_kb);
    
    // 执行测试用例
    TestPointExecutionResult executeTestCase(int test_case_id, const std::string& executable_path, const TestCase& testcase, int time_limit_ms, int memory_limit_kb);
    
    // 判断输出是否正确
    bool checkOutput(const std::string& output, const std::string& expected);
    
    // 清理临时文件
    void cleanup(const std::string& dir);
    
    // 创建临时目录
    std::string createTempDir(int submission_id);
    
    // 创建源代码文件
    std::string createSourceFile(const std::string& source_code, const std::string& language, const std::string& dir);
    
    // 获取编译命令
    std::string getCompileCommand(const std::string& source_file, const std::string& language);
    
    // 获取执行命令
    std::string getExecuteCommand(const std::string& executable_file, const std::string& language);
    
    // 获取评测结果的字符串描述
    std::string getJudgeResultString(JudgeResult result);
    
    // 将字符串可视化，显示不可见字符
    std::string visualizeString(const std::string& str);
    
    // 工作目录
    std::string work_dir_;
    
    // 时间限制（毫秒）
    int time_limit_ms_;
    
    // 内存限制（KB）
    int memory_limit_kb_;
};

#endif // JUDGE_ENGINE_H 