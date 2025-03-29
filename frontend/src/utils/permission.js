import router from '../router'
import store from '../store'
import { Message } from 'element-ui'

// 路由守卫
router.beforeEach((to, from, next) => {
  document.title = to.meta.title || '在线评测系统'
  
  const requiresAuth = to.matched.some(record => record.meta.requiresAuth)
  const requiresAdmin = to.matched.some(record => record.meta.requiresAdmin)
  const isAuthenticated = store.getters.isAuthenticated
  
  // 获取用户角色
  const userRole = store.state.user ? store.state.user.role : null
  const isAdmin = userRole >= 2 // 角色值>=2为管理员
  
  // 需要登录但未登录，重定向到登录页
  if (requiresAuth && !isAuthenticated) {
    Message.warning('请先登录')
    next('/login')
    return
  }
  
  // 需要管理员权限但用户不是管理员
  if (requiresAdmin && !isAdmin) {
    Message.warning('没有权限访问该页面')
    next('/user')
    return
  }
  
  // 已登录用户访问登录和注册页面，重定向到对应首页
  if (isAuthenticated && (to.path === '/login' || to.path === '/register')) {
    next(isAdmin ? '/admin' : '/user')
    return
  }
  
  // 已登录用户访问首页，根据角色重定向
  if (isAuthenticated && to.path === '/home') {
    next(isAdmin ? '/admin' : '/user')
    return
  }
  
  // 其他情况正常跳转
  next()
}) 