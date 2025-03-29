cmake_minimum_required(VERSION 3.10)
project(cplus_web_backend VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 添加包含目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 添加 MySQL 相关的包含目录和库目录
include_directories(/opt/homebrew/opt/mysql-client/include)
link_directories(/opt/homebrew/opt/mysql-client/lib)

# 添加 OpenSSL 的包含目录和库目录
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

# 添加 JsonCpp 的包含目录和库目录 - 直接指定路径
include_directories(/opt/homebrew/include)
link_directories(/opt/homebrew/lib)

# 添加 Crow 的包含目录
include_directories(${PROJECT_SOURCE_DIR}/lib/crow/include)

# 添加Boost依赖（Crow需要）
find_package(Boost COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# 寻找依赖库
find_package(Threads REQUIRED)

# 设置源代码文件
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 添加可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Threads::Threads
    mysqlclient  # 添加 MySQL 客户端库
    ${Boost_LIBRARIES}  # 添加 Boost 库
    ${OPENSSL_LIBRARIES}  # 添加 OpenSSL 库
    jsoncpp  # 添加 JsonCpp 库
)

# 让编译器支持C++17
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

# 安装目标
install(TARGETS ${PROJECT_NAME} DESTINATION bin) 