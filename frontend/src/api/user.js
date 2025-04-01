import request from '@/utils/request'

// 用户登录
export function login(data) {
  return request({
    url: '/api/user/login',
    method: 'post',
    data
  })
}

// 用户注册
export function register(data) {
  return request({
    url: '/api/user/register',
    method: 'post',
    data
  })
}

// 获取用户信息
export function getUserInfo() {
  return request({
    url: '/api/user/profile',
    method: 'get'
  })
}

// 用户登出
export function logout() {
  return request({
    url: '/api/user/logout',
    method: 'post'
  })
}

// ===== 管理员用户管理相关 API =====

// 获取用户列表
export function getUserList(offset = 0, limit = 10, search = '', role = '', status = '') {
  return request({
    url: '/api/admin/users',
    method: 'get',
    params: { offset, limit, search, role, status }
  })
}

// 创建用户
export function createUser(username, password, email, role) {
  return request({
    url: '/api/admin/users',
    method: 'post',
    data: { username, password, email, role }
  })
}

// 更新用户信息
export function updateUser(id, data) {
  return request({
    url: `/api/admin/users/${id}`,
    method: 'put',
    data
  })
}

// 删除用户
export function deleteUser(id) {
  return request({
    url: `/api/admin/users/${id}`,
    method: 'delete'
  })
}

// 更改用户角色
export function changeUserRole(id, role) {
  return request({
    url: `/api/admin/users/${id}/role`,
    method: 'post',
    data: { role }
  })
}

// 更改用户状态
export function changeUserStatus(id, status) {
  return request({
    url: `/api/admin/users/${id}/status`,
    method: 'post',
    data: { status }
  })
}

// 重置用户密码
export function resetUserPassword(id, new_password) {
  return request({
    url: `/api/admin/users/${id}/reset_password`,
    method: 'post',
    data: { new_password }
  })
} 