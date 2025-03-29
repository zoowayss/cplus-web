#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <string>
#include <functional>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <vector>
#include <memory>
#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <cstring>

namespace http {
    
    // HTTP请求结构
    struct Request {
        std::string method;
        std::string path;
        std::string http_version;
        std::map<std::string, std::string> headers;
        std::string body;
        
        std::string get_header(const std::string& key) const {
            auto it = headers.find(key);
            if (it != headers.end()) {
                return it->second;
            }
            return "";
        }
    };
    
    // HTTP响应结构
    struct Response {
        int status_code = 200;
        std::string status_message = "OK";
        std::map<std::string, std::string> headers;
        std::string body;
        
        void set_header(const std::string& key, const std::string& value) {
            headers[key] = value;
        }
        
        void set_content_type(const std::string& type) {
            set_header("Content-Type", type);
        }
        
        // 设置为JSON响应
        void json(const std::string& json_str) {
            body = json_str;
            set_content_type("application/json");
        }
        
        // 序列化响应为HTTP响应字符串
        std::string serialize() const {
            std::ostringstream oss;
            oss << "HTTP/1.1 " << status_code << " " << status_message << "\r\n";
            
            for (const auto& header : headers) {
                oss << header.first << ": " << header.second << "\r\n";
            }
            
            oss << "Content-Length: " << body.length() << "\r\n";
            oss << "\r\n";
            oss << body;
            
            return oss.str();
        }
    };
    
    // 路由处理器函数类型
    using RouteHandler = std::function<void(const Request&, Response&)>;
    
    // HTTP服务器类
    class HttpServer {
    private:
        uint16_t port_;
        int server_fd_;
        std::atomic<bool> running_;
        std::thread server_thread_;
        std::map<std::string, std::map<std::string, RouteHandler>> routes_;
        
        // 解析HTTP请求
        Request parse_request(const std::string& request_str) {
            Request req;
            std::istringstream iss(request_str);
            std::string line;
            
            // 解析请求行
            std::getline(iss, line);
            std::istringstream request_line(line);
            request_line >> req.method >> req.path >> req.http_version;
            
            // 解析头部
            while (std::getline(iss, line) && line != "\r") {
                size_t colon_pos = line.find(':');
                if (colon_pos != std::string::npos) {
                    std::string key = line.substr(0, colon_pos);
                    std::string value = line.substr(colon_pos + 1);
                    
                    // 去除前导空格
                    while (!value.empty() && isspace(value[0])) {
                        value.erase(0, 1);
                    }
                    
                    // 去除尾部回车符
                    if (!value.empty() && value.back() == '\r') {
                        value.pop_back();
                    }
                    
                    req.headers[key] = value;
                }
            }
            
            // 解析请求体
            // 如果Content-Length存在，则读取相应长度的内容
            std::string content_length_str = req.get_header("Content-Length");
            if (!content_length_str.empty()) {
                size_t content_length = std::stoul(content_length_str);
                std::vector<char> body_buffer(content_length);
                iss.read(body_buffer.data(), content_length);
                req.body = std::string(body_buffer.data(), content_length);
            }
            
            return req;
        }
        
        // 处理客户端连接
        void handle_client(int client_fd) {
            char buffer[8192] = {0};
            ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
            
            if (bytes_read > 0) {
                std::string request_str(buffer, bytes_read);
                Request request = parse_request(request_str);
                Response response;
                
                // 设置默认响应头
                response.set_header("Server", "C++ Simple HTTP Server");
                response.set_header("Connection", "close");
                
                // 查找匹配的路由处理器
                auto method_it = routes_.find(request.method);
                if (method_it != routes_.end()) {
                    auto route_it = method_it->second.find(request.path);
                    if (route_it != method_it->second.end()) {
                        // 调用处理器
                        route_it->second(request, response);
                    } else {
                        // 路径不存在
                        response.status_code = 404;
                        response.status_message = "Not Found";
                        response.json("{\"error\": \"Resource not found\"}");
                    }
                } else {
                    // 方法不支持
                    response.status_code = 405;
                    response.status_message = "Method Not Allowed";
                    response.json("{\"error\": \"Method not allowed\"}");
                }
                
                // 发送响应
                std::string response_str = response.serialize();
                send(client_fd, response_str.c_str(), response_str.length(), 0);
            }
            
            // 关闭客户端连接
            close(client_fd);
        }
        
        // 服务器主循环
        void server_loop() {
            while (running_) {
                // 设置监听超时，以便能够正常退出
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                
                // 设置超时以便能够检查running_标志
                fd_set read_fds;
                FD_ZERO(&read_fds);
                FD_SET(server_fd_, &read_fds);
                
                struct timeval timeout;
                timeout.tv_sec = 1;  // 1秒超时
                timeout.tv_usec = 0;
                
                int activity = select(server_fd_ + 1, &read_fds, NULL, NULL, &timeout);
                
                if (activity < 0 && errno != EINTR) {
                    std::cerr << "select error" << std::endl;
                    break;
                }
                
                if (activity > 0 && FD_ISSET(server_fd_, &read_fds)) {
                    int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &client_len);
                    if (client_fd < 0) {
                        std::cerr << "Accept failed" << std::endl;
                        continue;
                    }
                    
                    // 处理客户端请求
                    handle_client(client_fd);
                }
            }
        }
        
    public:
        HttpServer(uint16_t port = 8080) : port_(port), running_(false) {}
        
        ~HttpServer() {
            stop();
        }
        
        // 添加路由
        void add_route(const std::string& method, const std::string& path, RouteHandler handler) {
            routes_[method][path] = handler;
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
            // 创建服务器套接字
            server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
            if (server_fd_ < 0) {
                std::cerr << "Failed to create socket" << std::endl;
                return false;
            }
            
            // 设置套接字选项，允许端口重用
            int opt = 1;
            if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
                std::cerr << "Failed to set socket options" << std::endl;
                close(server_fd_);
                return false;
            }
            
            // 绑定端口
            struct sockaddr_in address;
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_port = htons(port_);
            
            if (bind(server_fd_, (struct sockaddr*)&address, sizeof(address)) < 0) {
                std::cerr << "Failed to bind to port " << port_ << std::endl;
                close(server_fd_);
                return false;
            }
            
            // 监听连接
            if (listen(server_fd_, 10) < 0) {
                std::cerr << "Failed to listen on socket" << std::endl;
                close(server_fd_);
                return false;
            }
            
            // 启动服务器线程
            running_ = true;
            server_thread_ = std::thread(&HttpServer::server_loop, this);
            std::cout << "HTTP服务器已启动，监听端口: " << port_ << std::endl;
            
            return true;
        }
        
        // 停止服务器
        void stop() {
            if (running_) {
                running_ = false;
                if (server_thread_.joinable()) {
                    server_thread_.join();
                }
                close(server_fd_);
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