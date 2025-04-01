import request from '@/utils/request'

// 获取排行榜数据
export function getLeaderboard(params) {
  // 确保所有参数都转换为字符串
  const queryParams = {}
  if (params) {
    Object.keys(params).forEach(key => {
      if (params[key] !== undefined && params[key] !== null) {
        queryParams[key] = String(params[key])
      }
    })
  }
  
  return request({
    url: '/api/leaderboard',
    method: 'get',
    params: queryParams
  })
} 