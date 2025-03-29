import axios from 'axios'
import { Message } from 'element-ui'
import store from '@/store'
import router from '@/router'

// 创建axios实例
const service = axios.create({
  baseURL: 'http://localhost:8080', // 直接连接到后端服务器
  timeout: 5000 // 请求超时
})

// 请求拦截器
service.interceptors.request.use(
  config => {
    // 请求头带上token
    if (store.getters.isAuthenticated) {
      // 移除token中可能存在的换行符
      const token = store.state.token.replace(/[\r\n]/g, '');
      config.headers['Authorization'] = `Bearer ${token}`;
    }
    return config
  },
  error => {
    console.error(error)
    return Promise.reject(error)
  }
)

// 响应拦截器
service.interceptors.response.use(
  response => {
    const res = response.data
    console.log('API响应:', res);
    
    // 请求成功 - 额外处理"正在评测"的特殊情况
    if (res.code === 200 || res.status === 'ok' || 
        (res.message && typeof res.message === 'string' && res.message.includes('正在评测'))) {
      return res
    } else {
      // 显示错误消息
      Message.error(res.message || '请求失败')
      
      // 401: 未登录或token过期
      if (res.code === 401) {
        store.dispatch('logout')
        router.push('/login')
      }
      
      return Promise.reject(new Error(res.message || '请求失败'))
    }
  },
  error => {
    console.error('响应错误: ', error)
    
    // 检查是否是评测相关的错误，如果是则认为是成功的
    const errorMessage = error.response?.data?.message || error.message || '';
    if (typeof errorMessage === 'string' && errorMessage.includes('正在评测')) {
      // 这是一个特殊情况，实际上是成功的
      return {
        status: 'ok',
        message: errorMessage,
        submission_id: Date.now() // 如果API没有返回submission_id，则生成一个临时ID
      };
    }
    
    Message.error(errorMessage || '网络错误')
    return Promise.reject(error)
  }
)

export default service 