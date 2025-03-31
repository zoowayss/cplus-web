#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <string>
#include <functional>
#include <map>
#include <memory>
#include <iostream>
#include <httplib.h>
#include <json/json.h>

namespace http {
    
    // HTTP请求结构的适配器
    struct Request {
        std::string method;
        std::string path;
        std::string http_version;
        std::map<std::string, std::string> headers;
        std::string body;
        
        // 从 httplib::Request 创建此适配器
        static Request from_httplib(const httplib::Request& req) {
            Request new_req;
            new_req.method = req.method;
            new_req.path = req.path;
            new_req.http_version = "HTTP/1.1"; // httplib 默认使用 HTTP/1.1
            new_req.body = req.body;
            
            // 复制所有 headers
            for (const auto& header : req.headers) {
                new_req.headers[header.first] = header.second;
            }
            
            return new_req;
        }
        
        std::string get_header(const std::string& key) const {
            auto it = headers.find(key);
            if (it != headers.end()) {
                return it->second;
            }
            return "";
        }
        
        // 获取不带查询参数的路径
        std::string get_base_path() const {
            size_t pos = path.find('?');
            if (pos != std::string::npos) {
                return path.substr(0, pos);
            }
            return path;
        }
    };
    
    // HTTP响应结构的适配器
    struct Response {
        int status_code = 200;
        std::string status_message = "OK";
        std::map<std::string, std::string> headers;
        std::string body;
        
        // 将此适配器应用到 httplib::Response
        void apply_to_httplib(httplib::Response& res) const {
            res.status = status_code;
            res.body = body;
            
            // 设置所有 headers
            for (const auto& header : headers) {
                res.set_header(header.first.c_str(), header.second.c_str());
            }
        }
        
        void set_header(const std::string& key, const std::string& value) {
            headers[key] = value;
        }
        
        void set_content_type(const std::string& type) {
            set_header("Content-Type", type);
        }
        
        // 设置CORS头部函数，允许跨域请求
        void set_cors_headers() {
            set_header("Access-Control-Allow-Origin", "*");
            set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS, PATCH");
            set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With, Accept, Origin");
            set_header("Access-Control-Allow-Credentials", "true");
            set_header("Access-Control-Max-Age", "3600"); // 缓存预检请求结果1小时
        }
        
        // 设置为JSON响应
        void json(const std::string& json_str) {
            body = json_str;
            set_content_type("application/json");
        }
    };
    
    // 路由处理器函数类型
    using RouteHandler = std::function<void(const Request&, Response&)>;
    
    // HTTP服务器适配器类
    class HttpServer {
    private:
        uint16_t port_;
        std::unique_ptr<httplib::Server> server_;
        
    public:
        // 修改构造函数，使用C++11兼容的方式创建unique_ptr
        HttpServer(uint16_t port = 8080) : port_(port), server_(new httplib::Server()) {
            // 设置错误处理程序
            server_->set_error_handler([](const httplib::Request& req, httplib::Response& res) {
                res.status = 500;
                res.set_content("{\"status\": \"error\", \"message\": \"Internal server error\"}", "application/json");
            });
            
            // 设置异常处理程序
            server_->set_exception_handler([](const httplib::Request& req, httplib::Response& res, std::exception_ptr ep) {
                try {
                    std::rethrow_exception(ep);
                } catch (const std::exception& e) {
                    std::cout << "Exception occurred: " << e.what() << std::endl;
                    res.status = 500;
                    res.set_content("{\"status\": \"error\", \"message\": \"Server exception occurred\"}", "application/json");
                }
            });
            
            // 配置 CORS，处理 OPTIONS 请求
            server_->Options(".*", [](const httplib::Request& req, httplib::Response& res) {
                res.set_header("Access-Control-Allow-Origin", "*");
                res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS, PATCH");
                res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With, Accept, Origin");
                res.set_header("Access-Control-Allow-Credentials", "true");
                res.set_header("Access-Control-Max-Age", "3600"); // 缓存预检请求结果1小时
                res.status = 204; // No Content
            });
        }
        
        ~HttpServer() {
            stop();
        }
        
        // 添加路由
        void add_route(const std::string& method, const std::string& path, RouteHandler handler) {
            auto wrapper = [handler](const httplib::Request& req, httplib::Response& res) {
                // 将 httplib 请求转换为我们的请求
                Request our_req = Request::from_httplib(req);
                Response our_res;
                
                // 设置默认响应头
                our_res.set_header("Server", "C++ HTTP Server");
                
                // 自动为所有响应添加CORS头部
                our_res.set_cors_headers();
                
                // 调用处理程序
                handler(our_req, our_res);
                
                // 将我们的响应应用到 httplib 响应
                our_res.apply_to_httplib(res);
            };
            
            // 注册到 httplib 服务器
            if (method == "GET") {
                server_->Get(path, wrapper);
            } else if (method == "POST") {
                server_->Post(path, wrapper);
            } else if (method == "PUT") {
                server_->Put(path, wrapper);
            } else if (method == "DELETE") {
                server_->Delete(path, wrapper);
            } else if (method == "OPTIONS") {
                server_->Options(path, wrapper);
            }
        }
        
        // GET方法路由
        void get(const std::string& path, RouteHandler handler) {
            add_route("GET", path, handler);
        }
        
        // POST方法路由
        void post(const std::string& path, RouteHandler handler) {
            add_route("POST", path, handler);
        }
        
        // PUT方法路由
        void put(const std::string& path, RouteHandler handler) {
            add_route("PUT", path, handler);
        }
        
        // DELETE方法路由
        void del(const std::string& path, RouteHandler handler) {
            add_route("DELETE", path, handler);
        }
        
        // 启动服务器
        bool start() {
            std::cout << "正在启动HTTP服务器，监听端口: " << port_ << std::endl;
            return server_->listen("0.0.0.0", port_);
        }
        
        // 停止服务器
        void stop() {
            if (server_) {
                server_->stop();
                std::cout << "HTTP服务器已停止" << std::endl;
            }
        }
        
        // 获取端口
        uint16_t port() const {
            return port_;
        }
    };
}

#endif // HTTP_SERVER_H 