# C++在线评测系统后端

这是一个C++实现的在线评测系统后端程序，使用C++11标准。

## 系统需求

- g++ 编译器(支持C++11)
- MySQL 客户端库
- OpenSSL

## 构建说明

本项目使用简单的Makefile构建系统，支持跨平台编译。确保您的系统上安装了g++编译器和必要的依赖库。

### 构建步骤

1. 进入后端目录：
```
cd backend
```

2. 编译项目：
```
make
```

编译成功后，可执行文件将生成在 `build/bin` 目录中，名为 `cplus_online_judge_backend`。

3. 运行程序：
```
make run
```
或直接运行：
```
./build/bin/cplus_online_judge_backend
```

4. 清理编译文件：
```
make clean
```

## 跨平台支持

Makefile会自动检测操作系统类型，根据不同平台设置正确的包含路径和库路径：

1. **macOS 支持**：
   - 自动查找 `/usr/local/include`, `/opt/homebrew/include` 等常见路径
   - 适配Homebrew安装的MySQL和OpenSSL库
   
2. **Linux 支持**：
   - 使用标准的 `/usr/include` 和 `/usr/lib` 路径
   - 包含 `/usr/lib/x86_64-linux-gnu` 以支持Debian/Ubuntu系统

如果您的MySQL或OpenSSL库安装在其他位置，您可以编辑Makefile，添加适合您系统的路径。

## Makefile说明

Makefile采用简化的格式，主要目标和命令如下：

```makefile
cplus_online_judge_backend: $(SRCS) $(JSONCPP_SRCS)
	g++ -o build/bin/$@ $^ -std=c++11 -Wall -pthread $(INCLUDES) $(MYSQL_LIB) $(OPENSSL_LIB)
```

这个简化的格式直接将所有源文件编译为最终的可执行文件，不需要生成中间的.o文件。

## 项目结构

- `src/`: 源代码目录
  - `main.cpp`: 程序入口
  - `controller/`: 控制器实现
  - `models/`: 数据模型
  - `services/`: 业务逻辑
  - `utils/`: 工具类
  - `database/`: 数据库封装
- `include/`: 头文件目录
- `lib/`: 第三方库
- `build/`: 构建输出目录

## 注意事项

- 本项目使用简化的Makefile构建系统
- 使用C++11标准，确保您的编译器支持
- 如果编译时找不到库文件，请在Makefile中添加您系统特定的MySQL和OpenSSL路径
- 如果编译时间较长，可考虑修改Makefile，添加增量编译功能 