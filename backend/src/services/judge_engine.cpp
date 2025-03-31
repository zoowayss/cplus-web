#include "../../include/services/judge_engine.h"
#include "../../include/services/submission_service.h"
#include "../../include/models/submission_repository.h"
#include "../../include/models/problem_repository.h"
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <mutex>
// 替换filesystem库，在C++11中不可用
// #include <filesystem>
#include <random>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <ctime>
#include <cerrno>
#include <cstring>

// 使用系统函数创建目录
bool createDirectory(const std::string& dir) {
    // 使用mkdir系统调用创建目录
    int status = mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (status == 0) {
        std::cout << "【评测引擎】成功创建目录: " << dir << std::endl;
        return true;
    } else if (errno == EEXIST) {
        // 目录已存在也视为成功
        std::cout << "【评测引擎】目录已存在: " << dir << std::endl;
        return true;
    }
    
    std::cerr << "【评测引擎】无法创建目录: " << dir << " - 错误: " << strerror(errno) << std::endl;
    return false;
}

// 检查目录是否可写
bool isDirectoryWritable(const std::string& dir) {
    std::string testFile = dir + "/test_write.tmp";
    FILE* fp = fopen(testFile.c_str(), "w");
    if (fp) {
        fclose(fp);
        remove(testFile.c_str());
        std::cout << "【评测引擎】目录可写: " << dir << std::endl;
        return true;
    }
    std::cerr << "【评测引擎】目录不可写: " << dir << " - 错误: " << strerror(errno) << std::endl;
    return false;
}

// 定义默认配置
#define DEFAULT_WORK_DIR "/tmp/judge_engine"
#define DEFAULT_TIME_LIMIT_MS 1000
#define DEFAULT_MEMORY_LIMIT_KB 262144 // 256MB

// 构造函数
JudgeEngine::JudgeEngine() 
    : work_dir_(DEFAULT_WORK_DIR), 
      time_limit_ms_(DEFAULT_TIME_LIMIT_MS), 
      memory_limit_kb_(DEFAULT_MEMORY_LIMIT_KB) {
    // 确保工作目录存在
    std::cout << "【评测引擎】初始化评测引擎，工作目录: " << work_dir_ << std::endl;
    
    if (!createDirectory(work_dir_)) {
        std::cerr << "【评测引擎】警告: 无法创建默认工作目录，尝试使用当前目录" << std::endl;
        // 尝试使用当前目录作为备选
        char buffer[1024];
        if (getcwd(buffer, sizeof(buffer)) != NULL) {
            work_dir_ = std::string(buffer) + "/judge_work";
            std::cout << "【评测引擎】使用备选工作目录: " << work_dir_ << std::endl;
            createDirectory(work_dir_);
        }
    }
    
    // 检查目录权限
    if (!isDirectoryWritable(work_dir_)) {
        std::cerr << "【评测引擎】严重错误: 工作目录不可写，评测功能可能无法正常工作" << std::endl;
    }
}

// 析构函数
JudgeEngine::~JudgeEngine() {
    // 清理可能的残留文件
    std::cout << "【评测引擎】评测引擎销毁，清理资源" << std::endl;
}

// 设置工作目录
void JudgeEngine::setWorkDir(const std::string& dir) {
    std::cout << "【评测引擎】设置工作目录: " << dir << std::endl;
    work_dir_ = dir;
    if (!createDirectory(work_dir_)) {
        std::cerr << "【评测引擎】无法设置工作目录，保持原目录: " << work_dir_ << std::endl;
    }
}

// 设置时间限制
void JudgeEngine::setTimeLimit(int ms) {
    std::cout << "【评测引擎】设置时间限制: " << ms << "ms" << std::endl;
    time_limit_ms_ = ms;
}

// 设置内存限制
void JudgeEngine::setMemoryLimit(int kb) {
    std::cout << "【评测引擎】设置内存限制: " << kb << "KB" << std::endl;
    memory_limit_kb_ = kb;
}

// 主评测函数
bool JudgeEngine::judge(int submission_id, std::string& error_message) {
    std::cout << "【评测引擎】开始评测提交ID: " << submission_id << std::endl;
    
    // 获取提交信息
    Submission submission = SubmissionService::getSubmissionInfo(submission_id);
    if (submission.getId() == 0) {
        error_message = "提交ID不存在";
        std::cerr << "【评测引擎】" << error_message << std::endl;
        return false;
    }
    
    // 获取题目信息
    Problem problem = ProblemRepository::getProblemById(submission.getProblemId());
    if (problem.getId() == 0) {
        error_message = "题目不存在";
        std::cerr << "【评测引擎】" << error_message << std::endl;
        return false;
    }
    
    // 更新提交状态为评测中
    SubmissionService::updateSubmissionStatus(submission_id, JudgeResult::JUDGING);
    
    // 获取测试用例
    std::vector<TestCase> testcases = ProblemRepository::getTestCasesByProblemId(problem.getId());
    if (testcases.empty()) {
        error_message = "题目没有测试用例";
        std::cerr << "【评测引擎】" << error_message << std::endl;
        SubmissionService::judgeSubmission(submission_id, JudgeResult::SYSTEM_ERROR, 0, 0, 0, error_message);
        return false;
    }
    std::cout << "【评测引擎】获取到 " << testcases.size() << " 个测试用例" << std::endl;
    
    // 创建临时目录
    std::string temp_dir = createTempDir(submission_id);
    if (temp_dir.empty()) {
        error_message = "创建临时目录失败";
        std::cerr << "【评测引擎】" << error_message << std::endl;
        SubmissionService::judgeSubmission(submission_id, JudgeResult::SYSTEM_ERROR, 0, 0, 0, error_message);
        return false;
    }
    std::cout << "【评测引擎】创建临时目录: " << temp_dir << std::endl;
    
    // 设置语言
    std::string language;
    switch (submission.getLanguage()) {
        case Language::CPP:
            language = "cpp";
            break;
        default:
            error_message = "不支持的语言";
            std::cerr << "【评测引擎】" << error_message << std::endl;
            SubmissionService::judgeSubmission(submission_id, JudgeResult::SYSTEM_ERROR, 0, 0, 0, error_message);
            cleanup(temp_dir);
            return false;
    }
    
    // 创建源文件
    std::string source_file = createSourceFile(submission.getSourceCode(), language, temp_dir);
    if (source_file.empty()) {
        error_message = "创建源文件失败";
        std::cerr << "【评测引擎】" << error_message << std::endl;
        SubmissionService::judgeSubmission(submission_id, JudgeResult::SYSTEM_ERROR, 0, 0, 0, error_message);
        cleanup(temp_dir);
        return false;
    }
    std::cout << "【评测引擎】创建源文件: " << source_file << std::endl;
    
    // 编译代码
    CompileResult compile_result = compileCode(submission.getSourceCode(), language, temp_dir);
    if (!compile_result.success) {
        // 更新编译错误信息
        std::cout << "【评测引擎】编译错误: " << compile_result.error_message << std::endl;
        SubmissionService::judgeSubmission(submission_id, JudgeResult::COMPILE_ERROR, 0, 0, 0, compile_result.error_message);
        cleanup(temp_dir);
        return true; // 编译错误也是一种正常的评测结果
    }
    std::cout << "【评测引擎】编译成功，可执行文件: " << compile_result.executable_path << std::endl;
    
    // 依次运行每个测试用例
    int total_cases = testcases.size();
    int passed_cases = 0;
    int total_time = 0;
    int max_memory = 0;
    JudgeResult final_result = JudgeResult::ACCEPTED;
    std::string failed_message;
    
    for (size_t i = 0; i < testcases.size(); i++) {
        TestCase& testcase = testcases[i];
        std::cout << "【评测引擎】运行测试用例 " << (i+1) << "/" << total_cases << ", ID: " << testcase.id << std::endl;
        
        // 记录测试用例输入到日志
        std::cout << "【评测引擎】测试用例 " << (i+1) << " 输入: \n" << testcase.input << std::endl;
        std::cout << "【评测引擎】测试用例 " << (i+1) << " 期望输出: \n" << testcase.expected_output << std::endl;
        
        // 执行测试用例
        TestPointExecutionResult test_result = executeTestCase(
            testcase.id, 
            compile_result.executable_path, 
            testcase, 
            problem.getTimeLimit() > 0 ? problem.getTimeLimit() : time_limit_ms_, 
            problem.getMemoryLimit() > 0 ? problem.getMemoryLimit() : memory_limit_kb_
        );
        
        // 输出详细的执行结果
        std::cout << "【评测引擎】测试用例 " << (i+1) << " 实际输出: \n" << test_result.output << std::endl;
        if (!test_result.error_message.empty()) {
            std::cout << "【评测引擎】测试用例 " << (i+1) << " 错误信息: \n" << test_result.error_message << std::endl;
        }
        
        // 更新统计信息
        total_time += test_result.time_used_ms;
        if (test_result.memory_used_kb > max_memory) {
            max_memory = test_result.memory_used_kb;
        }
        
        // 保存测试点结果
        TestPointResult point_result;
        point_result.setSubmissionId(submission_id);
        point_result.setTestCaseId(testcase.id);
        point_result.setResult(test_result.result);
        point_result.setPassed(test_result.passed);
        point_result.setTimeUsed(test_result.time_used_ms);
        point_result.setMemoryUsed(test_result.memory_used_kb);
        point_result.setOutput(test_result.output);
        point_result.setErrorMessage(test_result.error_message);
        
        SubmissionService::addTestResult(point_result);
        
        std::cout << "【评测引擎】测试用例 " << (i+1) << " 结果: " 
                  << (test_result.passed ? "通过" : "未通过") 
                  << ", 用时: " << test_result.time_used_ms << "ms" 
                  << ", 内存: " << test_result.memory_used_kb << "KB" << std::endl;
        
        // 如果该测试点通过
        if (test_result.passed) {
            passed_cases++;
        } 
        // 如果该测试点未通过且最终结果不是ACCEPTED
        else if (final_result == JudgeResult::ACCEPTED) {
            final_result = test_result.result;
            failed_message = test_result.error_message;
            
            // 输出详细的比较结果
            std::cout << "【评测引擎】测试用例 " << (i+1) << " 失败原因: " << getJudgeResultString(test_result.result) << std::endl;
            
            if (test_result.result == JudgeResult::WRONG_ANSWER) {
                // 转换输出为字符表示，方便查看不可见字符
                std::string vis_output = visualizeString(test_result.output);
                std::string vis_expected = visualizeString(testcase.expected_output);
                
                std::cout << "【评测引擎】输出对比(可视化):\n";
                std::cout << "- 实际输出: " << vis_output << std::endl;
                std::cout << "- 期望输出: " << vis_expected << std::endl;
            }
        }
    }
    
    // 计算得分（简单方式：通过率）
    int score = (passed_cases * 100) / total_cases;
    
    // 更新提交结果
    std::cout << "【评测引擎】评测完成，结果: " 
              << "通过数: " << passed_cases << "/" << total_cases 
              << ", 得分: " << score 
              << ", 最大用时: " << total_time << "ms" 
              << ", 最大内存: " << max_memory << "KB" << std::endl;
    
    SubmissionService::judgeSubmission(
        submission_id,
        final_result,
        score,
        total_time,
        max_memory,
        failed_message
    );
    
    // 清理临时文件
    cleanup(temp_dir);
    std::cout << "【评测引擎】清理临时目录: " << temp_dir << std::endl;
    
    return true;
}

// 编译代码
CompileResult JudgeEngine::compileCode(const std::string& source_code, const std::string& language, const std::string& compile_dir) {
    std::string source_file = createSourceFile(source_code, language, compile_dir);
    if (source_file.empty()) {
        return CompileResult(false, "创建源文件失败");
    }
    
    // 获取编译命令
    std::string compile_cmd = getCompileCommand(source_file, language);
    
    // 创建编译输出和错误文件
    std::string compile_output_file = compile_dir + "/compile_output.txt";
    std::string compile_error_file = compile_dir + "/compile_error.txt";
    
    // 构造完整命令（重定向输出和错误）
    std::string command = compile_cmd + " > " + compile_output_file + " 2> " + compile_error_file;
    
    // 执行编译命令
    int result = system(command.c_str());
    
    // 检查编译结果
    if (result != 0) {
        // 读取编译错误信息
        std::ifstream error_file(compile_error_file);
        std::stringstream error_stream;
        error_stream << error_file.rdbuf();
        std::string error_message = error_stream.str();
        
        // 限制错误消息长度（避免过长）
        if (error_message.length() > 1024) {
            error_message = error_message.substr(0, 1024) + "...";
        }
        
        return CompileResult(false, error_message);
    }
    
    // 构造可执行文件路径
    std::string executable_path;
    if (language == "cpp") {
        executable_path = compile_dir + "/solution";
    }
    
    // 检查可执行文件是否存在
    struct stat st;
    if (stat(executable_path.c_str(), &st) != 0) {
        return CompileResult(false, "编译似乎成功但未生成可执行文件");
    }
    
    return CompileResult(true, "", executable_path);
}

// 执行程序
ExecutionResult JudgeEngine::executeProgram(const std::string& executable_path, const std::string& input, int time_limit_ms, int memory_limit_kb) {
    ExecutionResult result;
    
    // 创建临时文件用于输入和输出
    char input_file_template[] = "/tmp/judge_input_XXXXXX";
    char output_file_template[] = "/tmp/judge_output_XXXXXX";
    char error_file_template[] = "/tmp/judge_error_XXXXXX";
    
    int input_fd = mkstemp(input_file_template);
    int output_fd = mkstemp(output_file_template);
    int error_fd = mkstemp(error_file_template);
    
    if (input_fd == -1 || output_fd == -1 || error_fd == -1) {
        result.success = false;
        result.result = JudgeResult::SYSTEM_ERROR;
        result.error_message = "无法创建临时文件";
        return result;
    }
    
    // 写入输入数据
    std::cout << "【评测引擎】写入测试用例输入: " << input << std::endl;
    write(input_fd, input.c_str(), input.length());
    write(input_fd, "\n", 1); // 确保输入有换行符结尾
    close(input_fd);
    
    // 创建子进程
    pid_t pid = fork();
    
    if (pid < 0) {
        // Fork失败
        result.success = false;
        result.result = JudgeResult::SYSTEM_ERROR;
        result.error_message = "无法创建子进程";
        return result;
    } 
    else if (pid == 0) {
        // 子进程
        
        // 重定向标准输入
        int fd_in = open(input_file_template, O_RDONLY);
        if (fd_in == -1) {
            exit(EXIT_FAILURE);
        }
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
        
        // 重定向标准输出
        dup2(output_fd, STDOUT_FILENO);
        close(output_fd);
        
        // 重定向标准错误
        dup2(error_fd, STDERR_FILENO);
        close(error_fd);
        
        // 设置资源限制
        struct rlimit time_limit;
        time_limit.rlim_cur = time_limit_ms / 1000 + 1; // 转换为秒，并加一秒缓冲
        time_limit.rlim_max = time_limit.rlim_cur + 1;
        setrlimit(RLIMIT_CPU, &time_limit);
        
        struct rlimit mem_limit;
        mem_limit.rlim_cur = memory_limit_kb * 1024; // 转换为字节
        mem_limit.rlim_max = mem_limit.rlim_cur;
        setrlimit(RLIMIT_AS, &mem_limit);
        
        // 执行程序
        execl(executable_path.c_str(), executable_path.c_str(), NULL);
        
        // 如果到达这里，说明执行失败
        exit(EXIT_FAILURE);
    } 
    else {
        // 父进程
        
        // 记录开始时间
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // 监控子进程
        int status;
        struct rusage resource_usage;
        
        // 等待子进程结束，并获取资源使用情况
        if (wait4(pid, &status, 0, &resource_usage) == -1) {
            result.success = false;
            result.result = JudgeResult::SYSTEM_ERROR;
            result.error_message = "等待子进程失败";
            return result;
        }
        
        // 记录结束时间
        auto end_time = std::chrono::high_resolution_clock::now();
        
        // 读取程序输出
        close(output_fd);
        std::ifstream output_file(output_file_template);
        std::stringstream output_stream;
        output_stream << output_file.rdbuf();
        result.output = output_stream.str();
        std::cout << "【评测引擎】程序输出: " << result.output << std::endl;
        
        // 读取错误输出
        close(error_fd);
        std::ifstream error_file(error_file_template);
        std::stringstream error_stream;
        error_stream << error_file.rdbuf();
        result.error_message = error_stream.str();
        if (!result.error_message.empty()) {
            std::cout << "【评测引擎】程序错误输出: " << result.error_message << std::endl;
        }
        
        // 计算资源使用情况
        result.time_used_ms = resource_usage.ru_utime.tv_sec * 1000 + resource_usage.ru_utime.tv_usec / 1000;
        
        // 在rusage测量不准确时使用chrono计时作为备选
        if (result.time_used_ms == 0) {
            result.time_used_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        }
        
        result.memory_used_kb = resource_usage.ru_maxrss;
        
        // 检查运行结果
        if (WIFEXITED(status)) {
            result.exit_code = WEXITSTATUS(status);
            
            if (result.exit_code == 0) {
                result.success = true;
                result.result = JudgeResult::ACCEPTED;
            } else {
                result.success = false;
                result.result = JudgeResult::RUNTIME_ERROR;
                result.error_message = "程序以非零状态码退出: " + std::to_string(result.exit_code);
            }
        } 
        else if (WIFSIGNALED(status)) {
            result.success = false;
            result.exit_code = -1;
            
            int signal = WTERMSIG(status);
            if (signal == SIGXCPU || result.time_used_ms >= time_limit_ms) {
                result.result = JudgeResult::TIME_LIMIT_EXCEEDED;
                result.error_message = "程序超出时间限制";
            } 
            else if (signal == SIGSEGV) {
                if (result.memory_used_kb >= memory_limit_kb) {
                    result.result = JudgeResult::MEMORY_LIMIT_EXCEEDED;
                    result.error_message = "程序超出内存限制";
                } else {
                    result.result = JudgeResult::RUNTIME_ERROR;
                    result.error_message = "段错误";
                }
            } 
            else {
                result.result = JudgeResult::RUNTIME_ERROR;
                result.error_message = "程序被信号终止: " + std::to_string(signal);
            }
        } 
        else {
            result.success = false;
            result.result = JudgeResult::SYSTEM_ERROR;
            result.error_message = "未知错误";
        }
        
        // 删除临时文件
        unlink(input_file_template);
        unlink(output_file_template);
        unlink(error_file_template);
        
        return result;
    }
}

// 执行测试用例
TestPointExecutionResult JudgeEngine::executeTestCase(int test_case_id, const std::string& executable_path, const TestCase& testcase, int time_limit_ms, int memory_limit_kb) {
    TestPointExecutionResult result;
    result.test_case_id = test_case_id;
    
    // 执行程序
    ExecutionResult exec_result = executeProgram(executable_path, testcase.input, time_limit_ms, memory_limit_kb);
    
    // 复制执行结果信息
    result.passed = exec_result.success;
    result.result = exec_result.result;
    result.output = exec_result.output;
    result.expected = testcase.expected_output;
    result.error_message = exec_result.error_message;
    result.time_used_ms = exec_result.time_used_ms;
    result.memory_used_kb = exec_result.memory_used_kb;
    
    // 如果程序正常运行，检查输出
    if (exec_result.success) {
        if (!checkOutput(exec_result.output, testcase.expected_output)) {
            result.passed = false;
            result.result = JudgeResult::WRONG_ANSWER;
            result.error_message = "输出与预期不符";
        }
    }
    
    return result;
}

// 判断输出是否正确
bool JudgeEngine::checkOutput(const std::string& output, const std::string& expected) {
    std::cout << "【评测引擎】原始输出对比:\n";
    std::cout << "- 实际输出 raw: \"" << output << "\"" << std::endl;
    std::cout << "- 期望输出 raw: \"" << expected << "\"" << std::endl;
    
    // 去除尾部空白字符
    auto trim = [](std::string str) {
        // 去除开头的空白字符
        while (!str.empty() && isspace(str.front())) {
            str.erase(str.begin());
        }
        // 去除末尾的空白字符
        while (!str.empty() && isspace(str.back())) {
            str.pop_back();
        }
        return str;
    };
    
    // 规范化输出（去除行尾空白，统一换行符）
    auto normalize = [&trim](const std::string& str) {
        std::istringstream stream(str);
        std::string line;
        std::string result;
        while (std::getline(stream, line)) {
            result += trim(line) + "\n";
        }
        return trim(result); // 再次trim整体结果，去除最后的换行符
    };
    
    // 规范化处理括号和逗号周围的空格
    auto normalizeArrayFormat = [](std::string str) {
        // 标准化括号和逗号的格式 (去除空格)
        std::string result;
        for (size_t i = 0; i < str.length(); i++) {
            if (str[i] == ' ') {
                // 跳过括号和逗号附近的空格
                if (i > 0 && (str[i-1] == '[' || str[i-1] == ',' || str[i-1] == ']')) {
                    continue;
                }
                if (i < str.length() - 1 && (str[i+1] == '[' || str[i+1] == ',' || str[i+1] == ']')) {
                    continue;
                }
            }
            result += str[i];
        }
        return result;
    };
    
    std::string norm_output = normalize(output);
    std::string norm_expected = normalize(expected);
    
    std::cout << "【评测引擎】标准化后输出对比:\n";
    std::cout << "- 标准化后实际输出: \"" << norm_output << "\"" << std::endl;
    std::cout << "- 标准化后期望输出: \"" << norm_expected << "\"" << std::endl;
    
    // 首先尝试常规比较
    if (norm_output == norm_expected) {
        return true;
    }
    
    // 如果常规比较失败，尝试数组格式的特殊处理
    std::string array_output = normalizeArrayFormat(norm_output);
    std::string array_expected = normalizeArrayFormat(norm_expected);
    
    std::cout << "【评测引擎】数组格式化后输出对比:\n";
    std::cout << "- 格式化后实际输出: \"" << array_output << "\"" << std::endl;
    std::cout << "- 格式化后期望输出: \"" << array_expected << "\"" << std::endl;
    
    return array_output == array_expected;
}

// 清理临时文件
void JudgeEngine::cleanup(const std::string& dir) {
    // 使用系统命令移除目录
    std::string command = "rm -rf " + dir;
    system(command.c_str());
}

// 创建临时目录
std::string JudgeEngine::createTempDir(int submission_id) {
    std::string dir = work_dir_ + "/submission_" + std::to_string(submission_id) + "_" + std::to_string(time(NULL));
    
    if (createDirectory(dir)) {
        return dir;
    }
    
    return "";
}

// 创建源代码文件
std::string JudgeEngine::createSourceFile(const std::string& source_code, const std::string& language, const std::string& dir) {
    std::string filename;
    if (language == "cpp") {
        filename = dir + "/solution.cpp";
    } else {
        return "";
    }
    
    try {
        std::ofstream source_file(filename);
        
        if (language == "cpp") {
            // 添加必要的头文件 - 保留最基本的，不过多添加
            source_file << "#include <iostream>\n"
                       << "#include <vector>\n"
                       << "#include <string>\n"
                       << "#include <algorithm>\n"
                       << "#include <sstream>\n"  // 添加sstream支持
                       << "using namespace std;\n\n";
            
            // 写入用户代码
            source_file << "// 用户提交的代码\n" << source_code << "\n\n";
            
            // 检查用户代码中是否已经包含main函数
            bool has_main = source_code.find("int main") != std::string::npos || 
                           source_code.find("void main") != std::string::npos;
            
            // 如果用户代码中没有main函数，添加main函数骨架
            if (!has_main) {
                std::cout << "【评测引擎】用户代码中未找到main函数，添加main函数骨架" << std::endl;
                
                // 识别常见方法
                bool has_two_sum = source_code.find("twoSum") != std::string::npos;
                
                source_file << "// 自动添加的main函数\n"
                           << "int main() {\n"
                           << "    Solution solution;\n";
                
                // 根据题目类型添加相应的测试代码
                if (has_two_sum) {
                    source_file << "    // 测试 twoSum\n"
                               << "    string input_line;\n"
                               << "    getline(cin, input_line);\n"
                               << "    \n"
                               << "    // 解析输入格式 \"nums = [3,2,4], target = 6\"\n"
                               << "    size_t bracketStart = input_line.find('[');\n"
                               << "    size_t bracketEnd = input_line.find(']');\n"
                               << "    size_t targetPos = input_line.find(\"target\");\n"
                               << "    \n"
                               << "    vector<int> nums;\n"
                               << "    if (bracketStart != string::npos && bracketEnd != string::npos) {\n"
                               << "        string numsStr = input_line.substr(bracketStart + 1, bracketEnd - bracketStart - 1);\n"
                               << "        stringstream ss(numsStr);\n"
                               << "        string item;\n"
                               << "        while (getline(ss, item, ',')) {\n"
                               << "            nums.push_back(stoi(item));\n"
                               << "        }\n"
                               << "    }\n"
                               << "    \n"
                               << "    int target = 0;\n"
                               << "    if (targetPos != string::npos) {\n"
                               << "        size_t equalPos = input_line.find('=', targetPos);\n"
                               << "        if (equalPos != string::npos) {\n"
                               << "            string targetStr = input_line.substr(equalPos + 1);\n"
                               << "            target = stoi(targetStr);\n"
                               << "        }\n"
                               << "    }\n"
                               << "    \n"
                               << "    vector<int> result = solution.twoSum(nums, target);\n"
                               << "    \n"
                               << "    // 输出结果\n"
                               << "    cout << \"[\";\n"
                               << "    for (int i = 0; i < result.size(); i++) {\n"
                               << "        cout << result[i];\n"
                               << "        if (i < result.size() - 1) cout << \",\";\n"
                               << "    }\n"
                               << "    cout << \"]\" << endl;\n";
                } else {
                    // 默认情况，添加通用的测试代码，从标准输入读取
                    source_file << "    // 通用测试代码\n"
                               << "    cout << \"请提供特定于此问题的main函数\" << endl;\n";
                }
                
                source_file << "    \n"
                           << "    return 0;\n"
                           << "}\n";
            }
        } else {
            // 非C++语言直接写入源代码
            source_file << source_code;
        }
        
        source_file.close();
        
        std::cout << "【评测引擎】成功创建源文件并添加必要的头文件和main函数" << std::endl;
        return filename;
    } catch (const std::exception& e) {
        std::cerr << "创建源文件失败: " << e.what() << std::endl;
        return "";
    }
}

// 获取编译命令
std::string JudgeEngine::getCompileCommand(const std::string& source_file, const std::string& language) {
    if (language == "cpp") {
        // 使用C++11标准
        return "g++ -std=c++11 -O2 -Wall -o " + 
               source_file.substr(0, source_file.size() - 4) + " " + source_file;
    }
    return "";
}

// 获取执行命令
std::string JudgeEngine::getExecuteCommand(const std::string& executable_file, const std::string& language) {
    return executable_file;
}

// 获取评测结果的字符串描述
std::string JudgeEngine::getJudgeResultString(JudgeResult result) {
    switch (result) {
        case JudgeResult::PENDING:
            return "等待评测";
        case JudgeResult::JUDGING:
            return "评测中";
        case JudgeResult::ACCEPTED:
            return "通过";
        case JudgeResult::WRONG_ANSWER:
            return "答案错误";
        case JudgeResult::TIME_LIMIT_EXCEEDED:
            return "超时";
        case JudgeResult::MEMORY_LIMIT_EXCEEDED:
            return "内存超限";
        case JudgeResult::RUNTIME_ERROR:
            return "运行时错误";
        case JudgeResult::COMPILE_ERROR:
            return "编译错误";
        case JudgeResult::SYSTEM_ERROR:
            return "系统错误";
        default:
            return "未知结果";
    }
}

// 将字符串可视化，显示不可见字符
std::string JudgeEngine::visualizeString(const std::string& str) {
    std::stringstream result;
    for (char c : str) {
        if (c == '\n') {
            result << "\\n";
        } else if (c == '\r') {
            result << "\\r";
        } else if (c == '\t') {
            result << "\\t";
        } else if (c == ' ') {
            result << "·";  // 显示空格为中点
        } else if (c < 32 || c > 126) {
            // 显示其他控制字符为十六进制
            result << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)(unsigned char)c;
        } else {
            result << c;
        }
    }
    return result.str();
} 