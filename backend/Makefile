# C++在线评测系统 - Makefile
# 使用g++编译器，指定C++11标准

# 项目名称和主要源文件
SRCS = $(shell find src -name "*.cpp")

# 跨平台设置
# 检测操作系统类型
UNAME_S := $(shell uname -s)
# 检测Linux发行版
IS_CENTOS := $(shell if [ -f /etc/centos-release ] || [ -f /etc/redhat-release ]; then echo 1; else echo 0; fi)

# MySQL设置
ifeq ($(UNAME_S),Darwin)
    # macOS
    MYSQL_INCLUDE = -I/usr/local/include -I/opt/homebrew/include -I/opt/homebrew/opt/mysql-client/include
    MYSQL_LIB = -L/usr/local/lib -L/opt/homebrew/lib -L/opt/homebrew/opt/mysql-client/lib -lmysqlclient
else ifeq ($(IS_CENTOS),1)
    # CentOS/RHEL
    MYSQL_INCLUDE = -I/usr/include/mysql
    MYSQL_LIB = -L/usr/lib64/mysql -L/usr/lib/mysql -lmysqlclient
else
    # 其他Linux
    MYSQL_INCLUDE = -I/usr/include
    MYSQL_LIB = -L/usr/lib -L/usr/lib/x86_64-linux-gnu -lmysqlclient
endif

# OpenSSL设置
ifeq ($(UNAME_S),Darwin)
    # macOS
    OPENSSL_INCLUDE = -I/usr/local/include -I/opt/homebrew/include -I/opt/homebrew/opt/openssl/include
    OPENSSL_LIB = -L/usr/local/lib -L/opt/homebrew/lib -L/opt/homebrew/opt/openssl/lib -lssl -lcrypto
else ifeq ($(IS_CENTOS),1)
    # CentOS/RHEL
    OPENSSL_INCLUDE = -I/usr/include/openssl
    OPENSSL_LIB = -L/usr/lib64 -L/usr/lib -lssl -lcrypto
else
    # 其他Linux
    OPENSSL_INCLUDE = -I/usr/include
    OPENSSL_LIB = -L/usr/lib -L/usr/lib/x86_64-linux-gnu -lssl -lcrypto
endif

# JSON 库设置
ifeq ($(UNAME_S),Darwin)
    # macOS
    JSON_INCLUDE = -I/usr/local/include -I/opt/homebrew/include
    JSON_LIB = -L/usr/local/lib -L/opt/homebrew/lib -ljsoncpp
else ifeq ($(IS_CENTOS),1)
    # CentOS/RHEL
    JSON_INCLUDE = -I/usr/include/jsoncpp
    JSON_LIB = -L/usr/lib64 -L/usr/lib -ljsoncpp
else
    # 其他Linux
    JSON_INCLUDE = -I/usr/include/jsoncpp
    JSON_LIB = -L/usr/lib -L/usr/lib/x86_64-linux-gnu -ljsoncpp
endif

# HTTP 库设置 - 使用系统安装的库而非lib目录中的库
# 在CentOS上安装依赖: sudo yum install jsoncpp-devel openssl-devel mysql-devel

# 包含目录 - 移除对lib目录的引用
INCLUDES = -Iinclude $(MYSQL_INCLUDE) $(OPENSSL_INCLUDE) $(JSON_INCLUDE)

# 主目标
cplus_online_judge_backend: $(SRCS)
	mkdir -p build/bin
	g++ -o build/bin/$@ $^ -std=c++11 -Wall -pthread $(INCLUDES) $(MYSQL_LIB) $(OPENSSL_LIB) $(JSON_LIB)

# 运行程序
run: cplus_online_judge_backend
	./build/bin/cplus_online_judge_backend

# 清理编译生成的文件
.PHONY: clean
clean:
	rm -rf build

# 默认目标
.DEFAULT_GOAL := cplus_online_judge_backend 