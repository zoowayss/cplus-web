import Vue from 'vue'
import VueRouter from 'vue-router'
import store from '../store'

// 基础页面
import Login from '../views/Login.vue'
import Register from '../views/Register.vue'
import Home from '../views/Home.vue'
import UserHome from '../views/UserHome.vue'

Vue.use(VueRouter)

// 解决Vue Router在3.1.0版本以上重复导航报错问题
const originalPush = VueRouter.prototype.push
const originalReplace = VueRouter.prototype.replace

// 重写push方法
VueRouter.prototype.push = function push(location) {
  return originalPush.call(this, location).catch(err => {
    if (err.name !== 'NavigationDuplicated') {
      return Promise.reject(err)
    }
    return Promise.resolve()
  })
}

// 重写replace方法
VueRouter.prototype.replace = function replace(location) {
  return originalReplace.call(this, location).catch(err => {
    if (err.name !== 'NavigationDuplicated') {
      return Promise.reject(err)
    }
    return Promise.resolve()
  })
}

const routes = [
  {
    path: '/',
    redirect: '/home'
  },
  {
    path: '/home',
    name: 'Home',
    component: () => import('../views/MainPage.vue'),
    meta: {
      title: '在线评测系统 - 首页'
    }
  },
  {
    path: '/login',
    name: 'Login',
    component: Login,
    meta: {
      title: '在线评测系统 - 登录'
    }
  },
  {
    path: '/register',
    name: 'Register',
    component: Register,
    meta: {
      title: '在线评测系统 - 注册'
    }
  },
  {
    path: '/admin',
    name: 'AdminHome',
    component: Home,
    meta: {
      title: '在线评测系统 - 管理后台',
      requiresAuth: true,
      requiresAdmin: true
    },
    children: [
      {
        path: 'problems',
        name: 'AdminProblemList',
        component: () => import('../views/admin/problems/ProblemList.vue'),
        meta: { title: '题目管理', requiresAuth: true, requiresAdmin: true }
      },
      {
        path: 'problems/create',
        name: 'AdminProblemCreate',
        component: () => import('../views/admin/problems/ProblemEdit.vue'),
        meta: { title: '创建题目', requiresAuth: true, requiresAdmin: true }
      },
      {
        path: 'problems/edit/:id',
        name: 'AdminProblemEdit',
        component: () => import('../views/admin/problems/ProblemEdit.vue'),
        meta: { title: '编辑题目', requiresAuth: true, requiresAdmin: true }
      },
      {
        path: 'problems/testcases/:id',
        name: 'AdminTestCases',
        component: () => import('../views/admin/problems/TestCaseManager.vue'),
        meta: { title: '测试用例管理', requiresAuth: true, requiresAdmin: true }
      }
    ]
  },
  {
    path: '/user',
    name: 'UserHome',
    component: UserHome,
    meta: {
      title: '在线评测系统 - 用户中心',
      requiresAuth: true
    }
  },
  // 题目管理相关路由
  {
    path: '/problems',
    name: 'ProblemList',
    component: () => import('../views/problems/ProblemList.vue'),
    meta: { title: '题目列表', requiresAuth: true }
  },
  {
    path: '/problems/:id',
    name: 'ProblemDetail',
    component: () => import('../views/problems/ProblemDetail.vue'),
    meta: { title: '题目详情', requiresAuth: true }
  },
  // // 讨论相关路由
  // {
  //   path: '/discussions/:id',
  //   name: 'DiscussionDetail',
  //   component: () => import('../views/discussions/DiscussionDetail.vue'),
  //   meta: { title: '讨论详情', requiresAuth: true }
  // },
  // 提交记录相关路由
  {
    path: '/submissions',
    name: 'SubmissionList',
    component: () => import('../views/problems/SubmissionList.vue'),
    meta: { title: '提交记录', requiresAuth: true }
  },
  // 排行榜相关路由
  {
    path: '/leaderboard',
    name: 'Leaderboard',
    component: () => import('../views/problems/Leaderboard.vue'),
    meta: { title: '排行榜', requiresAuth: true }
  },
  // 捕获所有路由并重定向
  {
    path: '*',
    redirect: '/'
  }
]

const router = new VueRouter({
  mode: 'history',
  base: '/',
  routes
})

export default router 