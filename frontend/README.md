# Cplus Web 前端项目

本项目是基于Vue2和Element UI的前端应用，使用Vite作为构建工具。

## 项目结构

```
frontend/
├── public/           # 静态资源目录
├── src/
│   ├── api/          # API请求
│   ├── assets/       # 资源文件
│   ├── components/   # 公共组件
│   ├── router/       # 路由配置
│   ├── store/        # Vuex状态管理
│   ├── utils/        # 工具函数
│   ├── views/        # 页面视图
│   ├── App.vue       # 根组件
│   └── main.js       # 入口文件
├── .env              # 环境变量
├── index.html        # HTML入口
├── package.json      # 项目依赖
└── vite.config.js    # Vite配置
```

## 开发环境

### 安装依赖

```bash
npm install
```

### 启动开发服务器

```bash
npm run dev
```

### 构建生产版本

```bash
npm run build
```

### 预览生产构建

```bash
npm run preview
```

## 功能特性

- 用户认证（登录/注册）
- 路由守卫
- 用户信息存储在LocalStorage
- 响应式设计
- Element UI组件库

## 后端API接口

- 登录: POST /api/user/login
- 注册: POST /api/user/register
- 获取用户信息: GET /api/user/info
- 登出: POST /api/user/logout 