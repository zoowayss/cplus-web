#include "../../include/models/problem_repository.h"
#include "../../include/database/database.h"
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <map>

// 简化的数据库结果集处理
struct ResultSet {
    MYSQL_RES* res;
    MYSQL_ROW row;
    std::map<std::string, int> field_map;
    
    ResultSet(MYSQL_RES* result) : res(result), row(nullptr) {
        // 初始化字段映射
        if (res) {
            MYSQL_FIELD* fields = mysql_fetch_fields(res);
            int num_fields = mysql_num_fields(res);
            for (int i = 0; i < num_fields; i++) {
                field_map[fields[i].name] = i;
            }
        }
    }
    
    ~ResultSet() {
        if (res) {
            mysql_free_result(res);
        }
    }
    
    bool next() {
        row = mysql_fetch_row(res);
        return row != nullptr;
    }
    
    int getInt(const std::string& field) {
        auto it = field_map.find(field);
        if (it == field_map.end() || !row[it->second]) {
            return 0;
        }
        return std::stoi(row[it->second]);
    }
    
    std::string getString(const std::string& field) {
        auto it = field_map.find(field);
        if (it == field_map.end() || !row[it->second]) {
            return "";
        }
        return row[it->second];
    }
    
    int64_t getInt64(const std::string& field) {
        auto it = field_map.find(field);
        if (it == field_map.end() || !row[it->second]) {
            return 0;
        }
        return std::stoll(row[it->second]);
    }
    
    bool getBoolean(const std::string& field) {
        auto it = field_map.find(field);
        if (it == field_map.end() || !row[it->second]) {
            return false;
        }
        return std::string(row[it->second]) == "1";
    }
};

// 根据ID获取题目信息
Problem ProblemRepository::getProblemById(int problem_id, bool include_test_cases) {
    Problem problem;
    try {
        // 构建查询
        std::string query = "SELECT * FROM problems WHERE id = ";
        query += std::to_string(problem_id);
        
        // 执行查询
        MYSQL_RES* result = Database::getInstance()->executeQuery(query);
        if (!result) {
            std::cerr << "查询失败" << std::endl;
            return problem;
        }
        
        // 处理结果
        ResultSet rs(result);
        if (rs.next()) {
            problem.setId(problem_id);
            problem.setTitle(rs.getString("title"));
            problem.setDescription(rs.getString("description"));
            problem.setInputFormat(rs.getString("input_format"));
            problem.setOutputFormat(rs.getString("output_format"));
            problem.setDifficulty(rs.getString("difficulty"));
            problem.setTimeLimit(rs.getInt("time_limit"));
            problem.setMemoryLimit(rs.getInt("memory_limit"));
            problem.setExampleInput(rs.getString("example_input"));
            problem.setExampleOutput(rs.getString("example_output"));
            problem.setHint(rs.getString("hint"));
            problem.setCodeTemplate(rs.getString("code_template"));
            problem.setCreatedBy(rs.getInt("created_by"));
            problem.setCreatedAt(rs.getInt64("created_at"));
            problem.setUpdatedAt(rs.getInt64("updated_at"));
            problem.setStatus(rs.getInt("status"));
            
            // 如果需要包含测试用例
            if (include_test_cases) {
                problem.setTestCases(getTestCasesByProblemId(problem_id));
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "获取题目信息失败: " << e.what() << std::endl;
    }
    
    return problem;
}

// 获取题目的测试用例
std::vector<TestCase> ProblemRepository::getTestCasesByProblemId(int problem_id) {
    std::vector<TestCase> testcases;
    try {
        // 构建查询
        std::string query = "SELECT * FROM testcases WHERE problem_id = ";
        query += std::to_string(problem_id);
        query += " ORDER BY id";
        
        // 执行查询
        MYSQL_RES* result = Database::getInstance()->executeQuery(query);
        if (!result) {
            std::cerr << "查询失败" << std::endl;
            return testcases;
        }
        
        // 处理结果
        ResultSet rs(result);
        while (rs.next()) {
            TestCase testcase;
            testcase.id = rs.getInt("id");
            testcase.problem_id = rs.getInt("problem_id");
            testcase.input = rs.getString("input");
            testcase.expected_output = rs.getString("expected_output");
            testcase.is_example = rs.getBoolean("is_example");
            testcase.created_at = rs.getInt64("created_at");
            
            testcases.push_back(testcase);
        }
    } catch (const std::exception& e) {
        std::cerr << "获取测试用例失败: " << e.what() << std::endl;
    }
    
    return testcases;
}

// 创建题目
bool ProblemRepository::createProblem(Problem& problem) {
    try {
        // 构建插入语句
        std::time_t now = std::time(nullptr);
        std::string query = "INSERT INTO problems (title, description, input_format, output_format, "
                          "difficulty, time_limit, memory_limit, example_input, example_output, hint, code_template, "
                          "created_by, created_at, updated_at, status) VALUES (";
        
        Database* db = Database::getInstance();
        
        // 添加字段值
        query += "'" + db->escapeString(problem.getTitle()) + "', ";
        query += "'" + db->escapeString(problem.getDescription()) + "', ";
        query += "'" + db->escapeString(problem.getInputFormat()) + "', ";
        query += "'" + db->escapeString(problem.getOutputFormat()) + "', ";
        query += "'" + db->escapeString(problem.getDifficulty()) + "', ";
        query += std::to_string(problem.getTimeLimit()) + ", ";
        query += std::to_string(problem.getMemoryLimit()) + ", ";
        query += "'" + db->escapeString(problem.getExampleInput()) + "', ";
        query += "'" + db->escapeString(problem.getExampleOutput()) + "', ";
        query += "'" + db->escapeString(problem.getHint()) + "', ";
        query += "'" + db->escapeString(problem.getCodeTemplate()) + "', ";
        query += std::to_string(problem.getCreatedBy()) + ", ";
        query += std::to_string(now) + ", ";
        query += std::to_string(now) + ", ";
        query += std::to_string(problem.getStatus());
        query += ")";
        
        // 执行查询
        bool success = db->executeCommand(query);
        if (success) {
            // 获取自增ID
            int problem_id = db->getLastInsertId();
            if (problem_id > 0) {
                problem.setId(problem_id);
                problem.setCreatedAt(now);
                problem.setUpdatedAt(now);
            }
        }
        
        return success;
    } catch (const std::exception& e) {
        std::cerr << "创建题目失败: " << e.what() << std::endl;
        return false;
    }
}

// 更新题目
bool ProblemRepository::updateProblem(const Problem& problem) {
    try {
        // 构建更新语句
        std::time_t now = std::time(nullptr);
        std::string query = "UPDATE problems SET ";
        
        Database* db = Database::getInstance();
        
        // 添加字段值
        query += "title = '" + db->escapeString(problem.getTitle()) + "', ";
        query += "description = '" + db->escapeString(problem.getDescription()) + "', ";
        query += "input_format = '" + db->escapeString(problem.getInputFormat()) + "', ";
        query += "output_format = '" + db->escapeString(problem.getOutputFormat()) + "', ";
        query += "difficulty = '" + db->escapeString(problem.getDifficulty()) + "', ";
        query += "time_limit = " + std::to_string(problem.getTimeLimit()) + ", ";
        query += "memory_limit = " + std::to_string(problem.getMemoryLimit()) + ", ";
        query += "example_input = '" + db->escapeString(problem.getExampleInput()) + "', ";
        query += "example_output = '" + db->escapeString(problem.getExampleOutput()) + "', ";
        query += "hint = '" + db->escapeString(problem.getHint()) + "', ";
        query += "code_template = '" + db->escapeString(problem.getCodeTemplate()) + "', ";
        query += "updated_at = " + std::to_string(now) + ", ";
        query += "status = " + std::to_string(problem.getStatus());
        query += " WHERE id = " + std::to_string(problem.getId());
        
        // 执行查询
        return db->executeCommand(query);
    } catch (const std::exception& e) {
        std::cerr << "更新题目失败: " << e.what() << std::endl;
        return false;
    }
}

// 删除题目
bool ProblemRepository::deleteProblem(int problem_id) {
    // 默认实现，返回false表示未实现
    return false;
}

// 获取题目列表
std::vector<Problem> ProblemRepository::getProblems(int offset, int limit, const std::string& search) {
    // 默认实现，返回空列表
    return std::vector<Problem>();
}

// 获取题目总数
int ProblemRepository::countProblems(const std::string& search) {
    // 默认实现，返回0
    return 0;
}

// 添加测试用例
bool ProblemRepository::addTestCase(int problem_id, TestCase& testcase) {
    // 默认实现，返回false表示未实现
    return false;
}

// 更新测试用例
bool ProblemRepository::updateTestCase(const TestCase& testcase) {
    // 默认实现，返回false表示未实现
    return false;
}

// 删除测试用例
bool ProblemRepository::deleteTestCase(int testcase_id) {
    // 默认实现，返回false表示未实现
    return false;
}

// 检查题目权限
bool ProblemRepository::checkProblemPermission(int problem_id, int user_id, int user_role) {
    // 默认实现，管理员有权限，普通用户无权限
    return user_role >= 2;
} 