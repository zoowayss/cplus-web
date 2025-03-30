#include "../../include/services/problem_service.h"
#include "../../include/database/database.h"
#include <iostream>
#include <ctime>
#include <sstream>
#include <mysql/mysql.h>
#include <memory>

// 自定义MySQL结果集智能指针包装器
class MySQLResultWrapper {
private:
    MYSQL_RES* result;
    
public:
    explicit MySQLResultWrapper(MYSQL_RES* res) : result(res) {}
    
    ~MySQLResultWrapper() {
        if (result) {
            mysql_free_result(result);
            result = nullptr;
        }
    }
    
    // 禁止复制
    MySQLResultWrapper(const MySQLResultWrapper&) = delete;
    MySQLResultWrapper& operator=(const MySQLResultWrapper&) = delete;
    
    // 允许移动
    MySQLResultWrapper(MySQLResultWrapper&& other) noexcept : result(other.result) {
        other.result = nullptr;
    }
    
    MySQLResultWrapper& operator=(MySQLResultWrapper&& other) noexcept {
        if (this != &other) {
            if (result) {
                mysql_free_result(result);
            }
            result = other.result;
            other.result = nullptr;
        }
        return *this;
    }
    
    // 访问原始指针
    MYSQL_RES* get() const {
        return result;
    }
    
    // 检查结果集是否有效
    bool isValid() const {
        return result != nullptr;
    }
    
    // 获取行
    MYSQL_ROW fetchRow() const {
        return result ? mysql_fetch_row(result) : nullptr;
    }
};

// 获取所有题目
std::vector<Problem> ProblemService::getAllProblems(int offset, int limit, const std::string& search) {
    std::vector<Problem> problems;
    Database* db = Database::getInstance();
    
    try {
        std::stringstream sql;
        sql << "SELECT id, title, description, input_format, output_format, difficulty, "
            << "time_limit, memory_limit, example_input, example_output, hint, "
            << "created_by, created_at, updated_at, status "
            << "FROM problems WHERE 1=1 ";
        
        if (!search.empty()) {
            sql << "AND (title LIKE '%" << search << "%' OR description LIKE '%" << search << "%') ";
        }
        
        sql << "ORDER BY id DESC LIMIT " << limit << " OFFSET " << offset;
        
        MYSQL_RES* rawResult = db->executeQuery(sql.str());
        if (!rawResult) {
            std::cerr << "获取题目列表查询失败" << std::endl;
            return problems;
        }
        
        // 使用RAII包装器管理结果集
        MySQLResultWrapper result(rawResult);
        
        MYSQL_ROW row;
        while ((row = result.fetchRow())) {
            Problem problem;
            int col = 0;
            
            problem.setId(std::stoi(row[col++]));
            problem.setTitle(row[col++] ? row[col-1] : "");
            problem.setDescription(row[col++] ? row[col-1] : "");
            problem.setInputFormat(row[col++] ? row[col-1] : "");
            problem.setOutputFormat(row[col++] ? row[col-1] : "");
            problem.setDifficulty(row[col++] ? row[col-1] : "中等");
            problem.setTimeLimit(std::stoi(row[col++]));
            problem.setMemoryLimit(std::stoi(row[col++]));
            problem.setExampleInput(row[col++] ? row[col-1] : "");
            problem.setExampleOutput(row[col++] ? row[col-1] : "");
            problem.setHint(row[col++] ? row[col-1] : "");
            problem.setCreatedBy(std::stoi(row[col++]));
            problem.setCreatedAt(std::stoll(row[col++]));
            problem.setUpdatedAt(std::stoll(row[col++]));
            problem.setStatus(std::stoi(row[col++]));
            
            problems.push_back(problem);
        }
    } catch (const std::exception& e) {
        std::cerr << "获取题目列表时发生异常: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "获取题目列表时发生未知异常" << std::endl;
    }
    
    return problems;
}

// 获取题目详情
Problem ProblemService::getProblemById(int problem_id, bool with_testcases) {
    Problem problem;
    Database* db = Database::getInstance();
    
    try {
        std::cout << "正在构建查询SQL..." << std::endl;
        std::stringstream sql;
        sql << "SELECT id, title, description, input_format, output_format, difficulty, "
            << "time_limit, memory_limit, example_input, example_output, hint, "
            << "created_by, created_at, updated_at, status "
            << "FROM problems WHERE id = " << problem_id;
        
        std::cout << "准备执行查询: " << sql.str() << std::endl;
        MYSQL_RES* rawResult = db->executeQuery(sql.str());
        if (!rawResult) {
            std::cerr << "查询题目详情失败，可能是数据库连接问题" << std::endl;
            return problem;
        }
        
        // 使用RAII包装器管理结果集
        MySQLResultWrapper result(rawResult);
        std::cout << "查询结果集已获取并包装" << std::endl;
        
        MYSQL_ROW row = result.fetchRow();
        if (row) {
            std::cout << "找到题目记录，开始解析..." << std::endl;
            int col = 0;
            
            problem.setId(std::stoi(row[col++]));
            problem.setTitle(row[col++] ? row[col-1] : "");
            problem.setDescription(row[col++] ? row[col-1] : "");
            problem.setInputFormat(row[col++] ? row[col-1] : "");
            problem.setOutputFormat(row[col++] ? row[col-1] : "");
            problem.setDifficulty(row[col++] ? row[col-1] : "中等");
            problem.setTimeLimit(std::stoi(row[col++]));
            problem.setMemoryLimit(std::stoi(row[col++]));
            problem.setExampleInput(row[col++] ? row[col-1] : "");
            problem.setExampleOutput(row[col++] ? row[col-1] : "");
            problem.setHint(row[col++] ? row[col-1] : "");
            problem.setCreatedBy(std::stoi(row[col++]));
            problem.setCreatedAt(std::stoll(row[col++]));
            problem.setUpdatedAt(std::stoll(row[col++]));
            problem.setStatus(std::stoi(row[col++]));
            
            std::cout << "题目基本信息解析完成: ID=" << problem.getId() << ", 标题=" << problem.getTitle() << std::endl;
        } else {
            std::cerr << "未找到ID为 " << problem_id << " 的题目" << std::endl;
            return problem;
        }
        
        // 如果需要获取测试用例，而且问题ID有效
        if (with_testcases && problem.getId() != 0) {
            std::cout << "开始获取题目的测试用例..." << std::endl;
            // 获取测试用例时使用独立的异常处理
            try {
                std::vector<TestCase> testcases = getTestCasesByProblemId(problem_id);
                problem.setTestCases(testcases);
                std::cout << "成功获取 " << testcases.size() << " 个测试用例" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "获取测试用例时发生错误: " << e.what() << std::endl;
                // 不重新抛出异常，允许返回没有测试用例的题目
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "获取题目详情时发生异常: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "获取题目详情时发生未知异常" << std::endl;
    }
    
    return problem;
}

// 创建题目
bool ProblemService::createProblem(const Problem& problem, std::string& error_message) {
    Database* db = Database::getInstance();
    
    // 当前时间戳
    int64_t now = std::time(nullptr);
    
    std::stringstream sql;
    sql << "INSERT INTO problems (title, description, input_format, output_format, difficulty, "
        << "time_limit, memory_limit, example_input, example_output, hint, "
        << "created_by, created_at, updated_at, status) VALUES ("
        << "'" << db->escapeString(problem.getTitle()) << "', "
        << "'" << db->escapeString(problem.getDescription()) << "', "
        << "'" << db->escapeString(problem.getInputFormat()) << "', "
        << "'" << db->escapeString(problem.getOutputFormat()) << "', "
        << "'" << db->escapeString(problem.getDifficulty()) << "', "
        << problem.getTimeLimit() << ", "
        << problem.getMemoryLimit() << ", "
        << "'" << db->escapeString(problem.getExampleInput()) << "', "
        << "'" << db->escapeString(problem.getExampleOutput()) << "', "
        << "'" << db->escapeString(problem.getHint()) << "', "
        << problem.getCreatedBy() << ", "
        << now << ", "
        << now << ", "
        << problem.getStatus() << ")";
    
    if (!db->executeCommand(sql.str())) {
        error_message = "创建题目失败";
        return false;
    }
    
    // 获取新插入的题目ID
    int problem_id = db->getLastInsertId();
    
    // 如果有测试用例，添加测试用例
    const std::vector<TestCase>& testcases = problem.getTestCases();
    for (const auto& testcase : testcases) {
        std::stringstream tc_sql;
        tc_sql << "INSERT INTO testcases (problem_id, input, expected_output, is_example, created_at) VALUES ("
               << problem_id << ", "
               << "'" << db->escapeString(testcase.input) << "', "
               << "'" << db->escapeString(testcase.expected_output) << "', "
               << (testcase.is_example ? "1" : "0") << ", "
               << now << ")";
        
        if (!db->executeCommand(tc_sql.str())) {
            error_message = "添加测试用例失败";
            // 不因为测试用例失败而回滚题目创建
            continue;
        }
    }
    
    return true;
}

// 更新题目
bool ProblemService::updateProblem(const Problem& problem, std::string& error_message) {
    Database* db = Database::getInstance();
    
    // 当前时间戳
    int64_t now = std::time(nullptr);
    
    std::stringstream sql;
    sql << "UPDATE problems SET "
        << "title = '" << db->escapeString(problem.getTitle()) << "', "
        << "description = '" << db->escapeString(problem.getDescription()) << "', "
        << "input_format = '" << db->escapeString(problem.getInputFormat()) << "', "
        << "output_format = '" << db->escapeString(problem.getOutputFormat()) << "', "
        << "difficulty = '" << db->escapeString(problem.getDifficulty()) << "', "
        << "time_limit = " << problem.getTimeLimit() << ", "
        << "memory_limit = " << problem.getMemoryLimit() << ", "
        << "example_input = '" << db->escapeString(problem.getExampleInput()) << "', "
        << "example_output = '" << db->escapeString(problem.getExampleOutput()) << "', "
        << "hint = '" << db->escapeString(problem.getHint()) << "', "
        << "updated_at = " << now << ", "
        << "status = " << problem.getStatus() << " "
        << "WHERE id = " << problem.getId();
    
    if (!db->executeCommand(sql.str())) {
        error_message = "更新题目失败";
        return false;
    }
    
    return true;
}

// 删除题目
bool ProblemService::deleteProblem(int problem_id, std::string& error_message) {
    Database* db = Database::getInstance();
    
    // 先检查题目是否存在
    std::stringstream check_sql;
    check_sql << "SELECT id FROM problems WHERE id = " << problem_id;
    
    MYSQL_RES* result = db->executeQuery(check_sql.str());
    bool exists = result && mysql_num_rows(result) > 0;
    
    if (result) {
        mysql_free_result(result);
    }
    
    if (!exists) {
        error_message = "题目不存在";
        return false;
    }
    
    std::cout << "开始删除题目 ID: " << problem_id << std::endl;
    
    // 开始事务
    if (!db->executeCommand("START TRANSACTION")) {
        error_message = "无法开始事务";
        std::cout << "开始事务失败" << std::endl;
        return false;
    }
    
    bool success = true;
    
    // 1. 先删除与题目相关的讨论回复
    std::stringstream discussion_replies_sql;
    discussion_replies_sql << "DELETE dr FROM discussion_replies dr "
                           << "INNER JOIN discussions d ON dr.discussion_id = d.id "
                           << "WHERE d.problem_id = " << problem_id;
    
    std::cout << "删除讨论回复: " << discussion_replies_sql.str() << std::endl;
    if (!db->executeCommand(discussion_replies_sql.str())) {
        std::cout << "删除讨论回复失败" << std::endl;
        success = false;
    }
    
    // 2. 删除与题目相关的讨论
    std::stringstream discussions_sql;
    discussions_sql << "DELETE FROM discussions WHERE problem_id = " << problem_id;
    
    std::cout << "删除讨论: " << discussions_sql.str() << std::endl;
    if (!db->executeCommand(discussions_sql.str())) {
        std::cout << "删除讨论失败" << std::endl;
        success = false;
    }
    
    // 3. 删除与题目相关的测试点结果
    std::stringstream test_results_sql;
    test_results_sql << "DELETE tpr FROM test_point_results tpr "
                     << "INNER JOIN submissions s ON tpr.submission_id = s.id "
                     << "WHERE s.problem_id = " << problem_id;
    
    std::cout << "删除测试点结果: " << test_results_sql.str() << std::endl;
    if (!db->executeCommand(test_results_sql.str())) {
        std::cout << "删除测试点结果失败" << std::endl;
        success = false;
    }
    
    // 4. 删除与题目相关的提交记录
    std::stringstream submissions_sql;
    submissions_sql << "DELETE FROM submissions WHERE problem_id = " << problem_id;
    
    std::cout << "删除提交记录: " << submissions_sql.str() << std::endl;
    if (!db->executeCommand(submissions_sql.str())) {
        std::cout << "删除提交记录失败" << std::endl;
        success = false;
    }
    
    // 5. 删除测试用例
    std::stringstream tc_sql;
    tc_sql << "DELETE FROM testcases WHERE problem_id = " << problem_id;
    
    std::cout << "删除测试用例: " << tc_sql.str() << std::endl;
    if (!db->executeCommand(tc_sql.str())) {
        std::cout << "删除测试用例失败" << std::endl;
        success = false;
    }
    
    // 6. 最后删除题目
    std::stringstream sql;
    sql << "DELETE FROM problems WHERE id = " << problem_id;
    
    std::cout << "删除题目: " << sql.str() << std::endl;
    if (!db->executeCommand(sql.str())) {
        std::cout << "删除题目失败" << std::endl;
        success = false;
    }
    
    // 如果所有操作都成功，提交事务；否则回滚
    if (success) {
        std::cout << "所有删除操作成功，提交事务" << std::endl;
        if (!db->executeCommand("COMMIT")) {
            error_message = "无法提交事务";
            std::cout << "提交事务失败" << std::endl;
            db->executeCommand("ROLLBACK");
            return false;
        }
        std::cout << "成功删除题目 ID: " << problem_id << std::endl;
        return true;
    } else {
        std::cout << "删除过程中出现错误，回滚事务" << std::endl;
        db->executeCommand("ROLLBACK");
        error_message = "删除题目失败，已回滚所有操作";
        return false;
    }
}

// 添加测试用例
bool ProblemService::addTestCase(int problem_id, const TestCase& testcase, std::string& error_message) {
    Database* db = Database::getInstance();
    
    // 当前时间戳
    int64_t now = std::time(nullptr);
    
    std::stringstream sql;
    sql << "INSERT INTO testcases (problem_id, input, expected_output, is_example, created_at) VALUES ("
        << problem_id << ", "
        << "'" << db->escapeString(testcase.input) << "', "
        << "'" << db->escapeString(testcase.expected_output) << "', "
        << (testcase.is_example ? "1" : "0") << ", "
        << now << ")";
    
    if (!db->executeCommand(sql.str())) {
        error_message = "添加测试用例失败";
        return false;
    }
    
    return true;
}

// 更新测试用例
bool ProblemService::updateTestCase(const TestCase& testcase, std::string& error_message) {
    Database* db = Database::getInstance();
    
    std::stringstream sql;
    sql << "UPDATE testcases SET "
        << "input = '" << db->escapeString(testcase.input) << "', "
        << "expected_output = '" << db->escapeString(testcase.expected_output) << "', "
        << "is_example = " << (testcase.is_example ? "1" : "0") << " "
        << "WHERE id = " << testcase.id;
    
    if (!db->executeCommand(sql.str())) {
        error_message = "更新测试用例失败";
        return false;
    }
    
    return true;
}

// 删除测试用例
bool ProblemService::deleteTestCase(int testcase_id, std::string& error_message) {
    Database* db = Database::getInstance();
    
    std::stringstream sql;
    sql << "DELETE FROM testcases WHERE id = " << testcase_id;
    
    if (!db->executeCommand(sql.str())) {
        error_message = "删除测试用例失败";
        return false;
    }
    
    return true;
}

// 获取题目的所有测试用例
std::vector<TestCase> ProblemService::getTestCasesByProblemId(int problem_id) {
    std::vector<TestCase> testcases;
    Database* db = Database::getInstance();
    
    try {
        std::cout << "构建测试用例查询SQL..." << std::endl;
        std::stringstream sql;
        sql << "SELECT id, problem_id, input, expected_output, is_example, created_at "
            << "FROM testcases WHERE problem_id = " << problem_id;
        
        std::cout << "执行测试用例查询: " << sql.str() << std::endl;
        MYSQL_RES* rawResult = db->executeQuery(sql.str());
        if (!rawResult) {
            std::cerr << "查询测试用例失败" << std::endl;
            return testcases;
        }
        
        // 使用RAII包装器管理结果集
        MySQLResultWrapper result(rawResult);
        std::cout << "测试用例查询结果集已获取并包装" << std::endl;
        
        MYSQL_ROW row;
        while ((row = result.fetchRow())) {
            TestCase testcase;
            testcase.id = std::stoi(row[0]);
            testcase.problem_id = std::stoi(row[1]);
            testcase.input = row[2] ? row[2] : "";
            testcase.expected_output = row[3] ? row[3] : "";
            testcase.is_example = std::stoi(row[4]) != 0;
            testcase.created_at = std::stoll(row[5]);
            
            testcases.push_back(testcase);
        }
        
        std::cout << "成功解析 " << testcases.size() << " 个测试用例" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "获取题目测试用例时发生异常: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "获取题目测试用例时发生未知异常" << std::endl;
    }
    
    return testcases;
}

// 获取题目的示例测试用例
std::vector<TestCase> ProblemService::getExampleTestCasesByProblemId(int problem_id) {
    std::vector<TestCase> testcases;
    Database* db = Database::getInstance();
    
    try {
        std::stringstream sql;
        sql << "SELECT id, problem_id, input, expected_output, is_example, created_at "
            << "FROM testcases WHERE problem_id = " << problem_id << " AND is_example = 1";
        
        MYSQL_RES* rawResult = db->executeQuery(sql.str());
        if (!rawResult) {
            return testcases;
        }
        
        // 使用RAII包装器管理结果集
        MySQLResultWrapper result(rawResult);
        
        MYSQL_ROW row;
        while ((row = result.fetchRow())) {
            TestCase testcase;
            testcase.id = std::stoi(row[0]);
            testcase.problem_id = std::stoi(row[1]);
            testcase.input = row[2] ? row[2] : "";
            testcase.expected_output = row[3] ? row[3] : "";
            testcase.is_example = true;
            testcase.created_at = std::stoll(row[5]);
            
            testcases.push_back(testcase);
        }
    } catch (const std::exception& e) {
        std::cerr << "获取题目示例测试用例时发生异常: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "获取题目示例测试用例时发生未知异常" << std::endl;
    }
    
    return testcases;
}

// 获取题目计数
int ProblemService::countProblems(const std::string& search) {
    int count = 0;
    Database* db = Database::getInstance();
    
    try {
        std::stringstream sql;
        sql << "SELECT COUNT(*) FROM problems WHERE 1=1 ";
        
        if (!search.empty()) {
            sql << "AND (title LIKE '%" << search << "%' OR description LIKE '%" << search << "%') ";
        }
        
        MYSQL_RES* rawResult = db->executeQuery(sql.str());
        if (!rawResult) {
            return 0;
        }
        
        // 使用RAII包装器管理结果集
        MySQLResultWrapper result(rawResult);
        
        MYSQL_ROW row = result.fetchRow();
        if (row) {
            count = std::stoi(row[0]);
        }
    } catch (const std::exception& e) {
        std::cerr << "获取题目计数时发生异常: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "获取题目计数时发生未知异常" << std::endl;
    }
    
    return count;
}

// 检查用户是否有权限操作题目（创建者或管理员）
bool ProblemService::checkProblemPermission(int problem_id, int user_id, int user_role) {
    // 管理员始终有权限
    if (user_role >= 2) {
        std::cout << "用户 " << user_id << " 是管理员（角色: " << user_role << "），有权限操作题目 " << problem_id << std::endl;
        return true;
    }
    
    // 普通用户检查是否是题目创建者
    Database* db = Database::getInstance();
    
    std::stringstream sql;
    sql << "SELECT created_by FROM problems WHERE id = " << problem_id;
    
    MYSQL_RES* result = db->executeQuery(sql.str());
    if (!result) {
        std::cout << "检查权限时查询失败: 题目 " << problem_id << " 可能不存在" << std::endl;
        return false;
    }
    
    MYSQL_ROW row = mysql_fetch_row(result);
    bool has_permission = false;
    
    if (row && row[0]) {
        int created_by = std::stoi(row[0]);
        has_permission = (created_by == user_id);
        
        std::cout << "题目 " << problem_id << " 的创建者是 " << created_by 
                  << "，当前用户 " << user_id 
                  << (has_permission ? " 有权限操作" : " 无权限操作") << std::endl;
    }
    
    mysql_free_result(result);
    return has_permission;
} 