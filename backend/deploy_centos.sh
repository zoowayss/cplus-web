#!/bin/bash
# CentOS部署脚本

# 确保必要的系统依赖已安装
echo "安装系统依赖..."
sudo yum install -y gcc-c++ make jsoncpp-devel openssl-devel mysql-devel

# 确保httplib目录存在
mkdir -p include/httplib

# 如果lib/cpp-httplib目录存在，从lib复制httplib.h
if [ -d "lib/cpp-httplib" ] && [ -f "lib/cpp-httplib/httplib.h" ]; then
    echo "从项目lib目录复制httplib.h..."
    cp lib/cpp-httplib/httplib.h include/httplib/
else
    # 如果没有lib/cpp-httplib，则尝试从网络获取
    echo "从GitHub下载httplib.h..."
    curl -s https://ghfast.top/https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h -o include/httplib/httplib.h
    
    if [ $? -ne 0 ]; then
        echo "警告: 无法从GitHub下载httplib.h，请手动复制到include/httplib/目录"
    fi
fi

# 查找并替换所有对lib目录的引用
echo "检查并修复源代码中对lib目录的引用..."

# 查找并替换所有对lib/jsoncpp的引用
find src include -type f -name "*.cpp" -o -name "*.h" | xargs -I{} sed -i 's|"../lib/jsoncpp/include/json/json.h"|<json/json.h>|g' {}
find src include -type f -name "*.cpp" -o -name "*.h" | xargs -I{} sed -i 's|"../../lib/jsoncpp/include/json/json.h"|<json/json.h>|g' {}

# 编译项目
echo "编译项目..."
make clean && make

echo "部署完成。如果编译成功，可执行文件位于build/bin/目录。" 