cmake_minimum_required(VERSION 3.10)
project(cplus_web_backend VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 添加包含目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 添加调试信息
message(STATUS "Project source dir: ${PROJECT_SOURCE_DIR}")
message(STATUS "CMake current binary dir: ${CMAKE_CURRENT_BINARY_DIR}")

# 添加 MySQL 相关的包含目录和库目录
include_directories(/opt/homebrew/opt/mysql-client/include)
link_directories(/opt/homebrew/opt/mysql-client/lib)
message(STATUS "MySQL include dir: /opt/homebrew/opt/mysql-client/include")
message(STATUS "MySQL lib dir: /opt/homebrew/opt/mysql-client/lib")

# 添加 OpenSSL 的包含目录和库目录
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")

# 添加 JsonCpp 的包含目录和库目录 - 直接指定路径
include_directories(/opt/homebrew/include)
link_directories(/opt/homebrew/lib)
message(STATUS "JsonCpp include dir: /opt/homebrew/include")
message(STATUS "JsonCpp lib dir: /opt/homebrew/lib")

# 添加 Crow 的包含目录
include_directories(${PROJECT_SOURCE_DIR}/lib/crow/include)
message(STATUS "Crow include dir: ${PROJECT_SOURCE_DIR}/lib/crow/include")

# 直接指定 Boost 路径
set(BOOST_INCLUDE_DIR "/opt/homebrew/anaconda3/include")
set(BOOST_LIB_DIR "/opt/homebrew/anaconda3/lib")
include_directories(${BOOST_INCLUDE_DIR})
link_directories(${BOOST_LIB_DIR})
message(STATUS "Boost include dir: ${BOOST_INCLUDE_DIR}")
message(STATUS "Boost lib dir: ${BOOST_LIB_DIR}")

# 寻找依赖库
find_package(Threads REQUIRED)

# 设置源代码文件 (包含所有src目录下的cpp文件，包括controller目录中的文件)
file(GLOB_RECURSE SOURCES "src/*.cpp")
message(STATUS "源文件列表: ${SOURCES}")

# 添加可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Threads::Threads
    mysqlclient  # 添加 MySQL 客户端库
    boost_system boost_filesystem  # 直接指定 Boost 库名称
    ${OPENSSL_LIBRARIES}  # 添加 OpenSSL 库
    jsoncpp  # 添加 JsonCpp 库
)

# 让编译器支持C++17
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

# 安装目标
install(TARGETS ${PROJECT_NAME} DESTINATION bin)