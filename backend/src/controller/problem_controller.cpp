#include "../../include/controller/problem_controller.h"
#include <iostream>
#include <regex>
#include <sstream>

// 辅助函数：从查询字符串解析参数
std::map<std::string, std::string> parseQueryParameters(const std::string& query_string) {
    std::map<std::string, std::string> params;
    std::istringstream iss(query_string);
    std::string token;
    
    // 按 & 分割查询字符串
    while (std::getline(iss, token, '&')) {
        std::size_t pos = token.find('=');
        if (pos != std::string::npos) {
            std::string key = token.substr(0, pos);
            std::string value = token.substr(pos + 1);
            params[key] = value;
        }
    }
    
    return params;
}

void ProblemController::registerRoutes(http::HttpServer* server) {
    // 题目列表路由 - 注意：路由应该包含查询参数的处理
    server->get("/api/problems", [this](const http::Request& req, http::Response& res) {
        std::cout << "Received request to /api/problems with path: " << req.path << std::endl;
        // 打开CORS
        res.set_cors_headers();
        this->handleGetProblems(req, res);
    });
    
    // 题目详情路由
    server->get("/api/problems/[0-9]+", [this](const http::Request& req, http::Response& res) {
        res.set_cors_headers();
        this->handleGetProblemDetail(req, res);
    });
    
    // 创建题目路由 - 需要管理员权限
    server->post("/api/problems", middleware::AuthMiddleware::protectWithRole([this](const http::Request& req, http::Response& res) {
        res.set_cors_headers();
        this->handleCreateProblem(req, res);
    }, 2)); // 角色2为管理员
    
    // 更新题目路由 - 需要管理员权限
    server->put("/api/problems/[0-9]+", middleware::AuthMiddleware::protectWithRole([this](const http::Request& req, http::Response& res) {
        res.set_cors_headers();
        this->handleUpdateProblem(req, res);
    }, 2));
    
    // 删除题目路由 - 需要管理员权限
    server->del("/api/problems/[0-9]+", middleware::AuthMiddleware::protectWithRole([this](const http::Request& req, http::Response& res) {
        res.set_cors_headers();
        this->handleDeleteProblem(req, res);
    }, 2));
    
    // 测试用例相关路由
    
    // 获取题目的测试用例
    server->get("/api/problems/[0-9]+/testcases", middleware::AuthMiddleware::protectWithRole([this](const http::Request& req, http::Response& res) {
        res.set_cors_headers();
        this->handleGetTestCases(req, res);
    }, 2));
    
    // 添加测试用例
    server->post("/api/problems/[0-9]+/testcases", middleware::AuthMiddleware::protectWithRole([this](const http::Request& req, http::Response& res) {
        res.set_cors_headers();
        this->handleAddTestCase(req, res);
    }, 2));
    
    // 更新测试用例
    server->put("/api/testcases/[0-9]+", middleware::AuthMiddleware::protectWithRole([this](const http::Request& req, http::Response& res) {
        res.set_cors_headers();
        this->handleUpdateTestCase(req, res);
    }, 2));
    
    // 删除测试用例
    server->del("/api/testcases/[0-9]+", middleware::AuthMiddleware::protectWithRole([this](const http::Request& req, http::Response& res) {
        res.set_cors_headers();
        this->handleDeleteTestCase(req, res);
    }, 2));
    
    // 提交代码路由
    server->post("/api/submissions", middleware::AuthMiddleware::protect([this](const http::Request& req, http::Response& res) {
        res.set_cors_headers();
        this->handleSubmitCode(req, res);
    }));
    
    // 添加一个通用路由来处理 /api/problems 并打印所有请求信息
    server->get("/api/.*", [](const http::Request& req, http::Response& res) {
        std::cout << "Catch-all route hit: " << req.path << std::endl;
        
        // 打印请求头
        std::cout << "Request headers:" << std::endl;
        for(const auto& header : req.headers) {
            std::cout << "  " << header.first << ": " << header.second << std::endl;
        }
        
        res.set_cors_headers();
        res.status_code = 200;
        res.body = "{\"status\":\"ok\",\"message\":\"API is running\"}";
    });
}

// 获取题目列表
void ProblemController::handleGetProblems(const http::Request& req, http::Response& res) {
    std::cout << "Handling GET Problems request..." << std::endl;
    
    // 解析分页参数
    int offset = 0, limit = 10;
    std::string search = "";
    std::string difficulty = "";
    std::string status = "";
    
    // 路径可能包含查询参数，打印整个路径进行调试
    std::cout << "Request path: " << req.path << std::endl;
    
    // 解析查询参数
    size_t query_pos = req.path.find('?');
    if (query_pos != std::string::npos) {
        std::string query_string = req.path.substr(query_pos + 1);
        std::cout << "Query string: " << query_string << std::endl;
        
        auto params = parseQueryParameters(query_string);
        
        // 打印所有查询参数
        std::cout << "Parsed parameters:" << std::endl;
        for (const auto& param : params) {
            std::cout << "  " << param.first << ": " << param.second << std::endl;
        }
        
        // 尝试获取偏移量参数
        auto offset_it = params.find("offset");
        if (offset_it != params.end()) {
            try {
                offset = std::stoi(offset_it->second);
                std::cout << "Using offset: " << offset << std::endl;
            } catch (std::exception& e) {
                std::cout << "Invalid offset parameter: " << offset_it->second << std::endl;
            }
        }
        
        // 尝试获取限制参数
        auto limit_it = params.find("limit");
        if (limit_it != params.end()) {
            try {
                limit = std::stoi(limit_it->second);
                // 限制最大查询数量为50
                if (limit > 50) limit = 50;
            } catch (std::exception& e) {
                // 忽略非数字参数
            }
        }
        
        // 尝试获取搜索参数
        auto search_it = params.find("search");
        if (search_it != params.end()) {
            search = search_it->second;
        }
        
        // 尝试获取难度参数
        auto difficulty_it = params.find("difficulty");
        if (difficulty_it != params.end()) {
            difficulty = difficulty_it->second;
        }
        
        // 尝试获取状态参数
        auto status_it = params.find("status");
        if (status_it != params.end()) {
            status = status_it->second;
        }
    } else {
        std::cout << "No query parameters found in path." << std::endl;
    }
    
    // 获取题目列表
    std::cout << "Retrieving problems with offset=" << offset << ", limit=" << limit << ", search=\"" << search << "\"" << std::endl;
    std::vector<Problem> problems = ProblemService::getAllProblems(offset, limit, search);
    std::cout << "Retrieved " << problems.size() << " problems" << std::endl;
    
    // 获取总数
    int total = ProblemService::countProblems(search);
    std::cout << "Total problem count: " << total << std::endl;
    
    // 构建响应
    Json::Value problemsJson(Json::arrayValue);
    for (const auto& problem : problems) {
        problemsJson.append(problem.toJson());
    }
    
    Json::Value data;
    data["problems"] = problemsJson;
    data["total"] = total;
    data["offset"] = offset;
    data["limit"] = limit;
    
    sendSuccessResponse(res, "获取题目列表成功", data);
    std::cout << "Response sent successfully" << std::endl;
}

// 获取题目详情
void ProblemController::handleGetProblemDetail(const http::Request& req, http::Response& res) {
    // 从URL中提取题目ID
    int problem_id = getIdFromPath(req.path, "/api/problems/");
    if (problem_id <= 0) {
        sendErrorResponse(res, "无效的题目ID", 400);
        return;
    }
    
    // 获取题目详情（包括示例测试用例）
    Problem problem = ProblemService::getProblemById(problem_id, true);
    
    if (problem.getId() == 0) {
        sendErrorResponse(res, "题目不存在", 404);
        return;
    }
    
    // 构建响应
    Json::Value data;
    data["problem"] = problem.toJson();
    
    sendSuccessResponse(res, "获取题目详情成功", data);
}

// 创建题目
void ProblemController::handleCreateProblem(const http::Request& req, http::Response& res) {
    // 从请求头获取令牌获取用户ID
    std::string auth_header = req.get_header("Authorization");
    std::string token = auth_header.substr(7);
    int user_id = JWT::getUserIdFromToken(token);
    
    if (user_id <= 0) {
        sendErrorResponse(res, "无效的用户ID", 401);
        return;
    }
    
    // 解析请求体
    Json::Value reqJson = parseRequestBody(req.body);
    
    if (!reqJson.isMember("title") || !reqJson.isMember("description")) {
        sendErrorResponse(res, "请提供标题和描述", 400);
        return;
    }
    
    // 创建题目对象
    Problem problem = Problem::fromJson(reqJson);
    problem.setCreatedBy(user_id);
    
    // 保存题目
    std::string error_message;
    if (ProblemService::createProblem(problem, error_message)) {
        // 构建响应
        Json::Value data;
        data["problem_id"] = problem.getId();
        
        sendSuccessResponse(res, "创建题目成功", data);
    } else {
        sendErrorResponse(res, error_message, 500);
    }
}

// 更新题目
void ProblemController::handleUpdateProblem(const http::Request& req, http::Response& res) {
    // 从URL中提取题目ID
    int problem_id = getIdFromPath(req.path, "/api/problems/");
    if (problem_id <= 0) {
        sendErrorResponse(res, "无效的题目ID", 400);
        return;
    }
    
    // 从请求头获取令牌获取用户ID和角色
    std::string auth_header = req.get_header("Authorization");
    std::string token = auth_header.substr(7);
    int user_id = JWT::getUserIdFromToken(token);
    int user_role = JWT::getUserRoleFromToken(token);
    
    if (user_id <= 0) {
        sendErrorResponse(res, "无效的用户ID", 401);
        return;
    }
    
    // 检查权限
    if (!ProblemService::checkProblemPermission(problem_id, user_id, user_role)) {
        sendErrorResponse(res, "没有权限修改此题目", 403);
        return;
    }
    
    // 获取当前题目
    Problem existingProblem = ProblemService::getProblemById(problem_id);
    if (existingProblem.getId() == 0) {
        sendErrorResponse(res, "题目不存在", 404);
        return;
    }
    
    // 解析请求体
    Json::Value reqJson = parseRequestBody(req.body);
    
    // 更新题目对象
    Problem updatedProblem = Problem::fromJson(reqJson);
    updatedProblem.setId(problem_id);
    updatedProblem.setCreatedBy(existingProblem.getCreatedBy());
    updatedProblem.setCreatedAt(existingProblem.getCreatedAt());
    
    // 保存更新后的题目
    std::string error_message;
    if (ProblemService::updateProblem(updatedProblem, error_message)) {
        // 构建响应
        Json::Value data;
        data["problem_id"] = problem_id;
        
        sendSuccessResponse(res, "更新题目成功", data);
    } else {
        sendErrorResponse(res, error_message, 500);
    }
}

// 删除题目
void ProblemController::handleDeleteProblem(const http::Request& req, http::Response& res) {
    // 从URL中提取题目ID
    int problem_id = getIdFromPath(req.path, "/api/problems/");
    if (problem_id <= 0) {
        sendErrorResponse(res, "无效的题目ID", 400);
        return;
    }
    
    // 从请求头获取令牌获取用户ID和角色
    std::string auth_header = req.get_header("Authorization");
    std::string token = auth_header.substr(7);
    int user_id = JWT::getUserIdFromToken(token);
    int user_role = JWT::getUserRoleFromToken(token);
    
    if (user_id <= 0) {
        sendErrorResponse(res, "无效的用户ID", 401);
        return;
    }
    
    // 检查权限
    if (!ProblemService::checkProblemPermission(problem_id, user_id, user_role)) {
        sendErrorResponse(res, "没有权限删除此题目", 403);
        return;
    }
    
    // 删除题目
    std::string error_message;
    if (ProblemService::deleteProblem(problem_id, error_message)) {
        sendSuccessResponse(res, "删除题目成功");
    } else {
        sendErrorResponse(res, error_message, 500);
    }
}

// 获取测试用例
void ProblemController::handleGetTestCases(const http::Request& req, http::Response& res) {
    // 从URL中提取题目ID
    int problem_id = getIdFromPath(req.path, "/api/problems/");
    if (problem_id <= 0) {
        sendErrorResponse(res, "无效的题目ID", 400);
        return;
    }
    
    // 从请求头获取令牌获取用户ID和角色
    std::string auth_header = req.get_header("Authorization");
    std::string token = auth_header.substr(7);
    int user_id = JWT::getUserIdFromToken(token);
    int user_role = JWT::getUserRoleFromToken(token);
    
    if (user_id <= 0) {
        sendErrorResponse(res, "无效的用户ID", 401);
        return;
    }
    
    // 检查权限
    if (!ProblemService::checkProblemPermission(problem_id, user_id, user_role)) {
        sendErrorResponse(res, "没有权限查看此题目的测试用例", 403);
        return;
    }
    
    // 获取测试用例
    std::vector<TestCase> testcases = ProblemService::getTestCasesByProblemId(problem_id);
    
    // 构建响应
    Json::Value testcasesJson(Json::arrayValue);
    for (const auto& testcase : testcases) {
        testcasesJson.append(testcase.toJson());
    }
    
    Json::Value data;
    data["testcases"] = testcasesJson;
    data["problem_id"] = problem_id;
    
    sendSuccessResponse(res, "获取测试用例成功", data);
}

// 添加测试用例
void ProblemController::handleAddTestCase(const http::Request& req, http::Response& res) {
    // 从URL中提取题目ID
    int problem_id = getIdFromPath(req.path, "/api/problems/");
    if (problem_id <= 0) {
        sendErrorResponse(res, "无效的题目ID", 400);
        return;
    }
    
    // 从请求头获取令牌获取用户ID和角色
    std::string auth_header = req.get_header("Authorization");
    std::string token = auth_header.substr(7);
    int user_id = JWT::getUserIdFromToken(token);
    int user_role = JWT::getUserRoleFromToken(token);
    
    if (user_id <= 0) {
        sendErrorResponse(res, "无效的用户ID", 401);
        return;
    }
    
    // 检查权限
    if (!ProblemService::checkProblemPermission(problem_id, user_id, user_role)) {
        sendErrorResponse(res, "没有权限为此题目添加测试用例", 403);
        return;
    }
    
    // 解析请求体
    Json::Value reqJson = parseRequestBody(req.body);
    
    if (!reqJson.isMember("input") || !reqJson.isMember("expected_output")) {
        sendErrorResponse(res, "请提供输入和期望输出", 400);
        return;
    }
    
    // 创建测试用例
    TestCase testcase;
    testcase.problem_id = problem_id;
    testcase.input = reqJson["input"].asString();
    testcase.expected_output = reqJson["expected_output"].asString();
    testcase.is_example = reqJson.isMember("is_example") ? reqJson["is_example"].asBool() : false;
    
    // 保存测试用例
    std::string error_message;
    if (ProblemService::addTestCase(problem_id, testcase, error_message)) {
        sendSuccessResponse(res, "添加测试用例成功");
    } else {
        sendErrorResponse(res, error_message, 500);
    }
}

// 更新测试用例
void ProblemController::handleUpdateTestCase(const http::Request& req, http::Response& res) {
    // 从URL中提取测试用例ID
    int testcase_id = getIdFromPath(req.path, "/api/testcases/");
    if (testcase_id <= 0) {
        sendErrorResponse(res, "无效的测试用例ID", 400);
        return;
    }
    
    // 解析请求体
    Json::Value reqJson = parseRequestBody(req.body);
    
    if (!reqJson.isMember("input") || !reqJson.isMember("expected_output")) {
        sendErrorResponse(res, "请提供输入和期望输出", 400);
        return;
    }
    
    // 创建测试用例
    TestCase testcase;
    testcase.id = testcase_id;
    testcase.input = reqJson["input"].asString();
    testcase.expected_output = reqJson["expected_output"].asString();
    testcase.is_example = reqJson.isMember("is_example") ? reqJson["is_example"].asBool() : false;
    
    // 保存测试用例
    std::string error_message;
    if (ProblemService::updateTestCase(testcase, error_message)) {
        sendSuccessResponse(res, "更新测试用例成功");
    } else {
        sendErrorResponse(res, error_message, 500);
    }
}

// 删除测试用例
void ProblemController::handleDeleteTestCase(const http::Request& req, http::Response& res) {
    // 从URL中提取测试用例ID
    int testcase_id = getIdFromPath(req.path, "/api/testcases/");
    if (testcase_id <= 0) {
        sendErrorResponse(res, "无效的测试用例ID", 400);
        return;
    }
    
    // 删除测试用例
    std::string error_message;
    if (ProblemService::deleteTestCase(testcase_id, error_message)) {
        sendSuccessResponse(res, "删除测试用例成功");
    } else {
        sendErrorResponse(res, error_message, 500);
    }
}

// 提交代码处理
void ProblemController::handleSubmitCode(const http::Request& req, http::Response& res) {
    // 解析请求体
    Json::Value reqJson = parseRequestBody(req.body);
    
    if (!reqJson.isMember("problem_id") || !reqJson.isMember("code") || !reqJson.isMember("language")) {
        sendErrorResponse(res, "请提供题目ID、代码和语言", 400);
        return;
    }
    
    // 这里应该调用代码评测服务，目前暂时返回模拟数据
    Json::Value data;
    data["submission_id"] = 1;
    data["status"] = "pending";
    
    sendSuccessResponse(res, "代码提交成功，正在评测", data);
}

// 从路径参数中获取ID
int ProblemController::getIdFromPath(const std::string& path, const std::string& prefix) {
    std::regex pattern(prefix + "([0-9]+)");
    std::smatch matches;
    
    if (std::regex_search(path, matches, pattern) && matches.size() > 1) {
        try {
            return std::stoi(matches[1].str());
        } catch (std::exception& e) {
            return -1;
        }
    }
    
    return -1;
} 