#include "../../include/services/problem_service.h"
#include "../../include/database/database.h"
#include <iostream>
#include <ctime>
#include <sstream>
#include <mysql/mysql.h>

// 获取所有题目
std::vector<Problem> ProblemService::getAllProblems(int offset, int limit, const std::string& search) {
    std::vector<Problem> problems;
    Database* db = Database::getInstance();
    
    std::stringstream sql;
    sql << "SELECT id, title, description, input_format, output_format, difficulty, "
        << "time_limit, memory_limit, example_input, example_output, hint, "
        << "created_by, created_at, updated_at, status "
        << "FROM problems WHERE 1=1 ";
    
    if (!search.empty()) {
        sql << "AND (title LIKE '%" << search << "%' OR description LIKE '%" << search << "%') ";
    }
    
    sql << "ORDER BY id DESC LIMIT " << limit << " OFFSET " << offset;
    
    MYSQL_RES* result = db->executeQuery(sql.str());
    if (result) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
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
        mysql_free_result(result);
    }
    
    return problems;
}

// 获取题目详情
Problem ProblemService::getProblemById(int problem_id, bool with_testcases) {
    Problem problem;
    Database* db = Database::getInstance();
    
    std::stringstream sql;
    sql << "SELECT id, title, description, input_format, output_format, difficulty, "
        << "time_limit, memory_limit, example_input, example_output, hint, "
        << "created_by, created_at, updated_at, status "
        << "FROM problems WHERE id = " << problem_id;
    
    MYSQL_RES* result = db->executeQuery(sql.str());
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row) {
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
            
            // 如果需要获取测试用例
            if (with_testcases) {
                problem.setTestCases(getTestCasesByProblemId(problem_id));
            }
        }
        mysql_free_result(result);
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
        error_message = "创建题目失败: " + std::string(mysql_error(db->getConnection()));
        return false;
    }
    
    // 获取新插入的题目ID
    int problem_id = mysql_insert_id(db->getConnection());
    
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
            error_message = "添加测试用例失败: " + std::string(mysql_error(db->getConnection()));
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
        error_message = "更新题目失败: " + std::string(mysql_error(db->getConnection()));
        return false;
    }
    
    return true;
}

// 删除题目
bool ProblemService::deleteProblem(int problem_id, std::string& error_message) {
    Database* db = Database::getInstance();
    
    // 先删除测试用例
    std::stringstream tc_sql;
    tc_sql << "DELETE FROM testcases WHERE problem_id = " << problem_id;
    
    if (!db->executeCommand(tc_sql.str())) {
        error_message = "删除测试用例失败: " + std::string(mysql_error(db->getConnection()));
        return false;
    }
    
    // 再删除题目
    std::stringstream sql;
    sql << "DELETE FROM problems WHERE id = " << problem_id;
    
    if (!db->executeCommand(sql.str())) {
        error_message = "删除题目失败: " + std::string(mysql_error(db->getConnection()));
        return false;
    }
    
    return true;
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
        error_message = "添加测试用例失败: " + std::string(mysql_error(db->getConnection()));
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
        error_message = "更新测试用例失败: " + std::string(mysql_error(db->getConnection()));
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
        error_message = "删除测试用例失败: " + std::string(mysql_error(db->getConnection()));
        return false;
    }
    
    return true;
}

// 获取题目的所有测试用例
std::vector<TestCase> ProblemService::getTestCasesByProblemId(int problem_id) {
    std::vector<TestCase> testcases;
    Database* db = Database::getInstance();
    
    std::stringstream sql;
    sql << "SELECT id, problem_id, input, expected_output, is_example, created_at "
        << "FROM testcases WHERE problem_id = " << problem_id;
    
    MYSQL_RES* result = db->executeQuery(sql.str());
    if (result) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            TestCase testcase;
            testcase.id = std::stoi(row[0]);
            testcase.problem_id = std::stoi(row[1]);
            testcase.input = row[2] ? row[2] : "";
            testcase.expected_output = row[3] ? row[3] : "";
            testcase.is_example = std::stoi(row[4]) != 0;
            testcase.created_at = std::stoll(row[5]);
            
            testcases.push_back(testcase);
        }
        mysql_free_result(result);
    }
    
    return testcases;
}

// 获取题目的示例测试用例
std::vector<TestCase> ProblemService::getExampleTestCasesByProblemId(int problem_id) {
    std::vector<TestCase> testcases;
    Database* db = Database::getInstance();
    
    std::stringstream sql;
    sql << "SELECT id, problem_id, input, expected_output, is_example, created_at "
        << "FROM testcases WHERE problem_id = " << problem_id << " AND is_example = 1";
    
    MYSQL_RES* result = db->executeQuery(sql.str());
    if (result) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            TestCase testcase;
            testcase.id = std::stoi(row[0]);
            testcase.problem_id = std::stoi(row[1]);
            testcase.input = row[2] ? row[2] : "";
            testcase.expected_output = row[3] ? row[3] : "";
            testcase.is_example = true;
            testcase.created_at = std::stoll(row[5]);
            
            testcases.push_back(testcase);
        }
        mysql_free_result(result);
    }
    
    return testcases;
}

// 获取题目计数
int ProblemService::countProblems(const std::string& search) {
    int count = 0;
    Database* db = Database::getInstance();
    
    std::stringstream sql;
    sql << "SELECT COUNT(*) FROM problems WHERE 1=1 ";
    
    if (!search.empty()) {
        sql << "AND (title LIKE '%" << search << "%' OR description LIKE '%" << search << "%') ";
    }
    
    MYSQL_RES* result = db->executeQuery(sql.str());
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row) {
            count = std::stoi(row[0]);
        }
        mysql_free_result(result);
    }
    
    return count;
}

// 检查用户是否有权限操作题目（创建者或管理员）
bool ProblemService::checkProblemPermission(int problem_id, int user_id, int user_role) {
    // 管理员角色可以操作任何题目
    if (user_role >= 2) {
        return true;
    }
    
    Database* db = Database::getInstance();
    
    std::stringstream sql;
    sql << "SELECT created_by FROM problems WHERE id = " << problem_id;
    
    MYSQL_RES* result = db->executeQuery(sql.str());
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row) {
            int created_by = std::stoi(row[0]);
            mysql_free_result(result);
            
            // 检查是否是创建者
            return created_by == user_id;
        }
        mysql_free_result(result);
    }
    
    return false;
} 