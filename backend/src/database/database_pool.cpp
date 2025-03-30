#include "../../include/database/database_pool.h"

// 静态成员初始化
DatabasePool* DatabasePool::instance = nullptr;
std::mutex DatabasePool::instanceMutex; 