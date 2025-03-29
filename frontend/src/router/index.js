import Vue from 'vue'
import VueRouter from 'vue-router'

Vue.use(VueRouter)

const routes = [
  {
    path: '/',
    redirect: '/login'
  },
  {
    path: '/login',
    name: 'Login',
    component: () => import('../views/Login.vue'),
    meta: { title: '登录', requiresAuth: false }
  },
  {
    path: '/register',
    name: 'Register',
    component: () => import('../views/Register.vue'),
    meta: { title: '注册', requiresAuth: false }
  },
  {
    path: '/admin',
    name: 'AdminHome',
    component: () => import('../views/Home.vue'),
    meta: { title: '管理中心', requiresAuth: true, requiresAdmin: true }
  },
  {
    path: '/user',
    name: 'UserHome',
    component: () => import('../views/UserHome.vue'),
    meta: { title: '学生中心', requiresAuth: true }
  },
  {
    path: '/home',
    name: 'Home',
    redirect: to => {
      // 这里不直接重定向，而是在路由守卫中根据用户角色决定
      return { path: '/user' }
    }
  },
  {
    path: '*',
    redirect: '/login'
  }
]

const router = new VueRouter({
  mode: 'history',
  base: import.meta.env.BASE_URL, // 使用Vite环境变量
  routes
})

export default router 