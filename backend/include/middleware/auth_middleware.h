#ifndef AUTH_MIDDLEWARE_H
#define AUTH_MIDDLEWARE_H

#include "../http/http_server.h"
#include "../utils/jwt.h"
#include <string>
#include <functional>

namespace middleware {
    
    class AuthMiddleware {
    public:
        // 验证令牌
        static bool verifyToken(const http::Request& req, http::Response& res) {
            // 从请求头获取令牌
            std::string auth_header = req.get_header("Authorization");
            
            if (auth_header.empty() || auth_header.substr(0, 7) != "Bearer ") {
                res.status_code = 401;
                res.status_message = "Unauthorized";
                res.json("{\"status\": \"error\", \"message\": \"未提供有效的认证令牌\"}");
                return false;
            }
            
            // 提取令牌
            std::string token = auth_header.substr(7);
            
            // 验证令牌
            std::map<std::string, std::string> payload;
            if (!JWT::verifyToken(token, payload)) {
                res.status_code = 401;
                res.status_message = "Unauthorized";
                res.json("{\"status\": \"error\", \"message\": \"认证令牌无效或已过期\"}");
                return false;
            }
            
            return true;
        }
        
        // 验证用户角色
        static bool checkRole(const http::Request& req, http::Response& res, int required_role) {
            // 首先验证令牌
            if (!verifyToken(req, res)) {
                return false;
            }
            
            // 从请求头获取令牌
            std::string auth_header = req.get_header("Authorization");
            std::string token = auth_header.substr(7);
            
            // 获取用户角色
            int role = JWT::getUserRoleFromToken(token);
            
            if (role < required_role) {
                res.status_code = 403;
                res.status_message = "Forbidden";
                res.json("{\"status\": \"error\", \"message\": \"没有足够权限执行此操作\"}");
                return false;
            }
            
            return true;
        }
        
        // 创建受保护的路由处理器
        static http::RouteHandler protect(http::RouteHandler handler) {
            return [handler](const http::Request& req, http::Response& res) {
                if (verifyToken(req, res)) {
                    handler(req, res);
                }
            };
        }
        
        // 创建受角色保护的路由处理器
        static http::RouteHandler protectWithRole(http::RouteHandler handler, int required_role) {
            return [handler, required_role](const http::Request& req, http::Response& res) {
                if (checkRole(req, res, required_role)) {
                    handler(req, res);
                }
            };
        }
    };
    
}

#endif // AUTH_MIDDLEWARE_H 