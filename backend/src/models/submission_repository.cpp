#include "../../include/models/submission_repository.h"
#include "../../include/database/database.h"
#include <iostream>
#include <sstream>
#include <mysql/mysql.h>

// 创建提交记录
bool SubmissionRepository::createSubmission(Submission& submission) {
    Database* db = Database::getInstance();
    
    std::stringstream sql;
    sql << "INSERT INTO submissions (user_id, problem_id, language, source_code, "
        << "result, score, time_used, memory_used, error_message, created_at) VALUES ("
        << submission.getUserId() << ", "
        << submission.getProblemId() << ", "
        << "'" << db->escapeString(submission.getLanguageStr()) << "', "
        << "'" << db->escapeString(submission.getSourceCode()) << "', "
        << static_cast<int>(submission.getResult()) << ", "
        << submission.getScore() << ", "
        << submission.getTimeUsed() << ", "
        << submission.getMemoryUsed() << ", "
        << "'" << db->escapeString(submission.getErrorMessage()) << "', "
        << time(nullptr) << ")";
    
    if (!db->executeCommand(sql.str())) {
        std::cerr << "创建提交记录失败: " << mysql_error(db->getConnection()) << std::endl;
        return false;
    }
    
    // 设置新创建的提交记录ID
    submission.setId(db->getLastInsertId());
    return true;
}

// 通过ID获取提交记录
Submission SubmissionRepository::getSubmissionById(int id, bool include_test_results) {
    Submission submission;
    Database* db = Database::getInstance();
    
    std::stringstream sql;
    sql << "SELECT id, user_id, problem_id, language, source_code, "
        << "result, score, time_used, memory_used, error_message, "
        << "created_at, judged_at FROM submissions WHERE id = " << id;
    
    MYSQL_RES* result = db->executeQuery(sql.str());
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row) {
            int col = 0;
            
            submission.setId(std::stoi(row[col++]));
            submission.setUserId(std::stoi(row[col++]));
            submission.setProblemId(std::stoi(row[col++]));
            if (row[col]) {
                submission.setLanguage(std::string(row[col]));
            }
            col++;
            if (row[col]) {
                submission.setSourceCode(row[col]);
            }
            col++;
            submission.setResult(static_cast<JudgeResult>(std::stoi(row[col++])));
            submission.setScore(std::stoi(row[col++]));
            submission.setTimeUsed(std::stoi(row[col++]));
            submission.setMemoryUsed(std::stoi(row[col++]));
            if (row[col]) {
                submission.setErrorMessage(row[col]);
            }
            col++;
            submission.setCreatedAt(std::stoll(row[col++]));
            // 判题时间可能为NULL
            if (row[col] != nullptr) {
                submission.setJudgedAt(std::stoll(row[col]));
            }
            
            // 如果需要获取测试点结果
            if (include_test_results) {
                std::stringstream test_sql;
                test_sql << "SELECT id, submission_id, test_point_id, result, "
                         << "time_used, memory_used, output FROM test_point_results "
                         << "WHERE submission_id = " << id;
                
                MYSQL_RES* test_result = db->executeQuery(test_sql.str());
                if (test_result) {
                    MYSQL_ROW test_row;
                    std::vector<TestPointResult> test_results;
                    
                    while ((test_row = mysql_fetch_row(test_result))) {
                        TestPointResult tpr;
                        int test_col = 0;
                        
                        tpr.setId(std::stoi(test_row[test_col++]));
                        tpr.setSubmissionId(std::stoi(test_row[test_col++]));
                        tpr.setTestCaseId(std::stoi(test_row[test_col++]));
                        tpr.setResult(static_cast<JudgeResult>(std::stoi(test_row[test_col++])));
                        tpr.setTimeUsed(std::stoi(test_row[test_col++]));
                        tpr.setMemoryUsed(std::stoi(test_row[test_col++]));
                        tpr.setOutput(test_row[test_col] ? test_row[test_col] : "");
                        
                        test_results.push_back(tpr);
                    }
                    
                    submission.setTestResults(test_results);
                    mysql_free_result(test_result);
                }
            }
        }
        mysql_free_result(result);
    }
    
    return submission;
}

// 更新提交记录信息
bool SubmissionRepository::updateSubmission(const Submission& submission) {
    Database* db = Database::getInstance();
    
    std::stringstream sql;
    sql << "UPDATE submissions SET "
        << "result = " << static_cast<int>(submission.getResult()) << ", "
        << "score = " << submission.getScore() << ", "
        << "time_used = " << submission.getTimeUsed() << ", "
        << "memory_used = " << submission.getMemoryUsed() << ", "
        << "error_message = '" << db->escapeString(submission.getErrorMessage()) << "', "
        << "judged_at = " << time(nullptr) << " "
        << "WHERE id = " << submission.getId();
    
    if (!db->executeCommand(sql.str())) {
        std::cerr << "更新提交记录失败: " << mysql_error(db->getConnection()) << std::endl;
        return false;
    }
    
    return true;
}

// 添加测试点结果
bool SubmissionRepository::addTestResult(int submission_id, TestPointResult& test_result) {
    Database* db = Database::getInstance();
    
    std::stringstream sql;
    sql << "INSERT INTO test_point_results (submission_id, test_point_id, result, "
        << "time_used, memory_used, output) VALUES ("
        << submission_id << ", "
        << test_result.getTestCaseId() << ", "
        << static_cast<int>(test_result.getResult()) << ", "
        << test_result.getTimeUsed() << ", "
        << test_result.getMemoryUsed() << ", "
        << "'" << db->escapeString(test_result.getOutput()) << "')";
    
    if (!db->executeCommand(sql.str())) {
        std::cerr << "添加测试点结果失败: " << mysql_error(db->getConnection()) << std::endl;
        return false;
    }
    
    // 设置新创建的测试结果ID
    test_result.setId(db->getLastInsertId());
    return true;
}

// 获取用户的所有提交记录
std::vector<Submission> SubmissionRepository::getSubmissionsByUserId(int user_id, int offset, int limit) {
    std::vector<Submission> submissions;
    Database* db = Database::getInstance();
    
    std::stringstream sql;
    sql << "SELECT s.id, s.user_id, s.problem_id, s.language, s.result, "
        << "s.score, s.time_used, s.memory_used, s.error_message, s.created_at, "
        << "s.judged_at, p.title as problem_title "
        << "FROM submissions s "
        << "LEFT JOIN problems p ON s.problem_id = p.id "
        << "WHERE s.user_id = " << user_id << " "
        << "ORDER BY s.id DESC LIMIT " << limit << " OFFSET " << offset;
    
    MYSQL_RES* result = db->executeQuery(sql.str());
    if (result) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            Submission submission;
            int col = 0;
            
            submission.setId(std::stoi(row[col++]));
            submission.setUserId(std::stoi(row[col++]));
            submission.setProblemId(std::stoi(row[col++]));
            if (row[col]) {
                submission.setLanguage(std::string(row[col]));
            }
            col++;
            submission.setResult(static_cast<JudgeResult>(std::stoi(row[col++])));
            submission.setScore(std::stoi(row[col++]));
            submission.setTimeUsed(std::stoi(row[col++]));
            submission.setMemoryUsed(std::stoi(row[col++]));
            if (row[col]) {
                submission.setErrorMessage(row[col]);
            }
            col++;
            submission.setCreatedAt(std::stoll(row[col++]));
            // 判题时间可能为NULL
            if (row[col] != nullptr) {
                submission.setJudgedAt(std::stoll(row[col]));
            }
            col++;
            
            // 设置题目标题
            std::string problem_title = row[col] ? row[col] : "";
            submission.setProblemTitle(problem_title);
            
            submissions.push_back(submission);
        }
        mysql_free_result(result);
    }
    
    return submissions;
}

// 获取题目的所有提交记录
std::vector<Submission> SubmissionRepository::getSubmissionsByProblemId(int problem_id, int offset, int limit) {
    std::vector<Submission> submissions;
    Database* db = Database::getInstance();
    
    std::stringstream sql;
    sql << "SELECT s.id, s.user_id, s.problem_id, s.language, s.source_code, "
        << "s.result, s.score, s.time_used, s.memory_used, s.error_message, "
        << "s.created_at, s.judged_at, u.username as username "
        << "FROM submissions s "
        << "LEFT JOIN users u ON s.user_id = u.id "
        << "WHERE s.problem_id = " << problem_id << " "
        << "ORDER BY s.id DESC LIMIT " << limit << " OFFSET " << offset;
    
    MYSQL_RES* result = db->executeQuery(sql.str());
    if (result) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            Submission submission;
            int col = 0;
            
            submission.setId(std::stoi(row[col++]));
            submission.setUserId(std::stoi(row[col++]));
            submission.setProblemId(std::stoi(row[col++]));
            submission.setLanguage(row[col++] ? row[col-1] : "cpp");
            // 不加载完整的源代码以减少数据传输量
            col++; // 跳过source_code
            submission.setResult(static_cast<JudgeResult>(std::stoi(row[col++])));
            submission.setScore(std::stoi(row[col++]));
            submission.setTimeUsed(std::stoi(row[col++]));
            submission.setMemoryUsed(std::stoi(row[col++]));
            submission.setErrorMessage(row[col++] ? row[col-1] : "");
            submission.setCreatedAt(std::stoll(row[col++]));
            // 判题时间可能为NULL
            if (row[col] != nullptr) {
                submission.setJudgedAt(std::stoll(row[col]));
            }
            col++;
            
            // 设置用户名
            std::string username = row[col] ? row[col] : "";
            submission.setUsername(username);
            
            submissions.push_back(submission);
        }
        mysql_free_result(result);
    }
    
    return submissions;
}

// 获取用户在特定题目的所有提交记录
std::vector<Submission> SubmissionRepository::getSubmissionsByUserAndProblem(int user_id, int problem_id, int offset, int limit) {
    std::vector<Submission> submissions;
    Database* db = Database::getInstance();
    
    std::stringstream sql;
    sql << "SELECT id, user_id, problem_id, language, source_code, "
        << "result, score, time_used, memory_used, error_message, "
        << "created_at, judged_at FROM submissions "
        << "WHERE user_id = " << user_id << " AND problem_id = " << problem_id << " "
        << "ORDER BY id DESC LIMIT " << limit << " OFFSET " << offset;
    
    MYSQL_RES* result = db->executeQuery(sql.str());
    if (result) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            Submission submission;
            int col = 0;
            
            submission.setId(std::stoi(row[col++]));
            submission.setUserId(std::stoi(row[col++]));
            submission.setProblemId(std::stoi(row[col++]));
            submission.setLanguage(row[col++] ? row[col-1] : "cpp");
            submission.setSourceCode(row[col++] ? row[col-1] : "");
            submission.setResult(static_cast<JudgeResult>(std::stoi(row[col++])));
            submission.setScore(std::stoi(row[col++]));
            submission.setTimeUsed(std::stoi(row[col++]));
            submission.setMemoryUsed(std::stoi(row[col++]));
            submission.setErrorMessage(row[col++] ? row[col-1] : "");
            submission.setCreatedAt(std::stoll(row[col++]));
            // 判题时间可能为NULL
            if (row[col] != nullptr) {
                submission.setJudgedAt(std::stoll(row[col]));
            }
            
            submissions.push_back(submission);
        }
        mysql_free_result(result);
    }
    
    return submissions;
}

// 获取所有提交记录
std::vector<Submission> SubmissionRepository::getAllSubmissions(int offset, int limit) {
    std::vector<Submission> submissions;
    Database* db = Database::getInstance();
    
    std::stringstream sql;
    sql << "SELECT s.id, s.user_id, s.problem_id, s.language, "
        << "s.result, s.score, s.time_used, s.memory_used, s.error_message, "
        << "s.created_at, s.judged_at, u.username as username, p.title as problem_title "
        << "FROM submissions s "
        << "LEFT JOIN users u ON s.user_id = u.id "
        << "LEFT JOIN problems p ON s.problem_id = p.id "
        << "ORDER BY s.id DESC LIMIT " << limit << " OFFSET " << offset;
    
    MYSQL_RES* result = db->executeQuery(sql.str());
    if (result) {
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            Submission submission;
            int col = 0;
            
            submission.setId(std::stoi(row[col++]));
            submission.setUserId(std::stoi(row[col++]));
            submission.setProblemId(std::stoi(row[col++]));
            submission.setLanguage(row[col++] ? row[col-1] : "cpp");
            submission.setResult(static_cast<JudgeResult>(std::stoi(row[col++])));
            submission.setScore(std::stoi(row[col++]));
            submission.setTimeUsed(std::stoi(row[col++]));
            submission.setMemoryUsed(std::stoi(row[col++]));
            submission.setErrorMessage(row[col++] ? row[col-1] : "");
            submission.setCreatedAt(std::stoll(row[col++]));
            // 判题时间可能为NULL
            if (row[col] != nullptr) {
                submission.setJudgedAt(std::stoll(row[col]));
            }
            col++;
            
            // 设置用户名和题目标题
            std::string username = row[col++] ? row[col-1] : "";
            std::string problem_title = row[col] ? row[col] : "";
            submission.setUsername(username);
            submission.setProblemTitle(problem_title);
            
            submissions.push_back(submission);
        }
        mysql_free_result(result);
    }
    
    return submissions;
}

// 获取提交记录总数
int SubmissionRepository::getSubmissionCount() {
    Database* db = Database::getInstance();
    
    std::string sql = "SELECT COUNT(*) FROM submissions";
    MYSQL_RES* result = db->executeQuery(sql);
    
    int count = 0;
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && row[0]) {
            count = std::stoi(row[0]);
        }
        mysql_free_result(result);
    }
    
    return count;
}

// 获取用户的提交记录总数
int SubmissionRepository::getSubmissionCountByUserId(int user_id) {
    Database* db = Database::getInstance();
    
    std::stringstream sql;
    sql << "SELECT COUNT(*) FROM submissions WHERE user_id = " << user_id;
    
    MYSQL_RES* result = db->executeQuery(sql.str());
    
    int count = 0;
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && row[0]) {
            count = std::stoi(row[0]);
        }
        mysql_free_result(result);
    }
    
    return count;
}

// 获取题目的提交记录总数
int SubmissionRepository::getSubmissionCountByProblemId(int problem_id) {
    Database* db = Database::getInstance();
    
    std::stringstream sql;
    sql << "SELECT COUNT(*) FROM submissions WHERE problem_id = " << problem_id;
    
    MYSQL_RES* result = db->executeQuery(sql.str());
    
    int count = 0;
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && row[0]) {
            count = std::stoi(row[0]);
        }
        mysql_free_result(result);
    }
    
    return count;
}

// 获取用户特定题目的提交记录总数
int SubmissionRepository::getSubmissionCountByUserAndProblemId(int user_id, int problem_id) {
    Database* db = Database::getInstance();
    
    std::stringstream sql;
    sql << "SELECT COUNT(*) FROM submissions WHERE user_id = " << user_id
        << " AND problem_id = " << problem_id;
    
    MYSQL_RES* result = db->executeQuery(sql.str());
    
    int count = 0;
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && row[0]) {
            count = std::stoi(row[0]);
        }
        mysql_free_result(result);
    }
    
    return count;
}

// 获取用户的通过题目数量
int SubmissionRepository::getAcceptedProblemCountByUserId(int user_id) {
    Database* db = Database::getInstance();
    
    // 查询用户通过的不同题目数量
    std::stringstream sql;
    sql << "SELECT COUNT(DISTINCT problem_id) FROM submissions "
        << "WHERE user_id = " << user_id << " AND result = 0"; // 0代表通过
    
    MYSQL_RES* result = db->executeQuery(sql.str());
    
    int count = 0;
    if (result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row && row[0]) {
            count = std::stoi(row[0]);
        }
        mysql_free_result(result);
    }
    
    return count;
} 