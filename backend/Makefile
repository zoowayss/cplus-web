# C++在线评测系统 - Makefile
# 使用g++编译器，指定C++11标准

# 项目名称和主要源文件
SRCS = $(shell find src -name "*.cpp")
JSONCPP_SRCS = lib/jsoncpp/src/lib_json/json_reader.cpp \
               lib/jsoncpp/src/lib_json/json_value.cpp \
               lib/jsoncpp/src/lib_json/json_writer.cpp

# 跨平台设置
# 检测操作系统类型
UNAME_S := $(shell uname -s)

# MySQL设置
ifeq ($(UNAME_S),Darwin)
    # macOS
    MYSQL_INCLUDE = -I/usr/local/include -I/opt/homebrew/include -I/opt/homebrew/opt/mysql-client/include
    MYSQL_LIB = -L/usr/local/lib -L/opt/homebrew/lib -L/opt/homebrew/opt/mysql-client/lib -lmysqlclient
else
    # Linux
    MYSQL_INCLUDE = -I/usr/include
    MYSQL_LIB = -L/usr/lib -L/usr/lib/x86_64-linux-gnu -lmysqlclient
endif

# OpenSSL设置
ifeq ($(UNAME_S),Darwin)
    # macOS
    OPENSSL_INCLUDE = -I/usr/local/include -I/opt/homebrew/include -I/opt/homebrew/opt/openssl/include
    OPENSSL_LIB = -L/usr/local/lib -L/opt/homebrew/lib -L/opt/homebrew/opt/openssl/lib -lssl -lcrypto
else
    # Linux
    OPENSSL_INCLUDE = -I/usr/include
    OPENSSL_LIB = -L/usr/lib -L/usr/lib/x86_64-linux-gnu -lssl -lcrypto
endif

# 包含目录
INCLUDES = -Iinclude -Ilib/cpp-httplib -Ilib/jsoncpp/include $(MYSQL_INCLUDE) $(OPENSSL_INCLUDE)

# 主目标
cplus_online_judge_backend: $(SRCS) $(JSONCPP_SRCS)
	mkdir -p build/bin
	g++ -o build/bin/$@ $^ -std=c++11 -Wall -pthread $(INCLUDES) $(MYSQL_LIB) $(OPENSSL_LIB)

# 运行程序
run: cplus_online_judge_backend
	./build/bin/cplus_online_judge_backend

# 清理编译生成的文件
.PHONY: clean
clean:
	rm -rf build

# 默认目标
.DEFAULT_GOAL := cplus_online_judge_backend 