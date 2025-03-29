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

  // 防止无限重定向循环
  // 如果当前路径是 /admin 并且带有 fromAdmin 查询参数，允许直接通过
  if (to.path === '/admin' && to.query.fromAdmin) {
    next()
    return
  }
  
  // 处理路径中的特殊情况 - 支持单页面应用结构
  const path = to.path
  
  // 防止重定向循环
  // 如果来源是管理员主页且查询参数中有component=problem-detail，说明我们已经在单页应用中了
  // 这种情况下不应该再次重定向
  if (from.path === '/admin' && from.query.component === 'problem-detail' && 
      path.match(/^\/problems\/\d+$/)) {
    next(false) // 取消导航
    return
  }
  
  // 同样检查用户主页
  if (from.path === '/user' && from.query.component === 'problem-detail' && 
      path.match(/^\/problems\/\d+$/)) {
    next(false) // 取消导航
    return
  }
  
  // 特殊处理：允许直接导航到管理员题目编辑页面
  if (isAuthenticated && isAdmin && path.match(/^\/admin\/problems\/edit\/\d+$/)) {
    next() // 直接通过导航
    return
  }
  
  // 特殊处理：允许直接导航到管理员测试用例管理页面
  if (isAuthenticated && isAdmin && path.match(/^\/admin\/problems\/testcases\/\d+$/)) {
    next() // 直接通过导航
    return
  }
  
  // 现在我们使用嵌套路由，不再需要重定向管理员子路由
  // 但如果是非标准子路由，仍需重定向到主页
  if (isAuthenticated && isAdmin && path.startsWith('/admin/') && path !== '/admin' && !to.query.fromAdmin) {
    // 检查是否是管理员合法的子路由
    const isValidAdminSubroute = [
      '/admin/problems',
      '/admin/problems/create',
      '/admin/users',
      '/admin/submissions',
      '/admin/settings'
    ].includes(path) || 
    path.match(/^\/admin\/problems\/edit\/\d+$/) || 
    path.match(/^\/admin\/problems\/testcases\/\d+$/);
    
    if (!isValidAdminSubroute) {
      next('/admin');
      return;
    }
  }
  
  // 处理题目详情页请求，重定向到相应主页并传递参数
  if (isAuthenticated && path.match(/^\/problems\/\d+$/)) {
    const problemId = path.split('/')[2] // 提取题目ID
    
    // 根据用户角色决定重定向路径
    if (isAdmin) {
      // 使用弹窗方式而不是完全替换页面
      next({
        path: '/admin',
        query: { component: 'problem-detail', problemId }
      })
    } else {
      next({
        path: '/user',
        query: { component: 'problem-detail', problemId }
      })
    }
    return
  }
  
  // 处理题目列表页请求，重定向到相应主页并加载题目列表
  if (isAuthenticated && path === '/problems') {
    if (isAdmin) {
      next({
        path: '/admin',
        query: { component: 'problems' }
      })
    } else {
      next({
        path: '/user',
        query: { component: 'problems' }
      })
    }
    return
  }
  
  // 处理提交记录页请求，重定向到相应主页并加载提交记录
  if (isAuthenticated && path === '/submissions') {
    if (isAdmin) {
      next({
        path: '/admin',
        query: { component: 'submissions' }
      })
    } else {
      next({
        path: '/user',
        query: { component: 'submissions' }
      })
    }
    return
  }
  
  // 处理排行榜页请求，重定向到相应主页并加载排行榜
  if (isAuthenticated && path === '/leaderboard') {
    if (isAdmin) {
      next({
        path: '/admin',
        query: { component: 'leaderboard' }
      })
    } else {
      next({
        path: '/user',
        query: { component: 'leaderboard' }
      })
    }
    return
  }
  
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