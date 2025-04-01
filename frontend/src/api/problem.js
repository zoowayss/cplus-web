import request from '@/utils/request'

// 获取题目列表
export function getProblems(params) {
  // 确保所有参数都转换为字符串
  const queryParams = {}
  if (params) {
    Object.keys(params).forEach(key => {
      if (params[key] !== undefined && params[key] !== null) {
        queryParams[key] = String(params[key])
      }
    })
  }
  
  console.log('Requesting problems with params:', queryParams)
  
  return request({
    url: '/api/problems',
    method: 'get',
    params: queryParams
  })
}

// 获取题目详情
export function getProblemDetail(id) {
  return request({
    url: `/api/problems/${id}`,
    method: 'get'
  })
}

// 创建题目
export function createProblem(data) {
  return request({
    url: '/api/problems',
    method: 'post',
    data
  })
}

// 更新题目
export function updateProblem(id, data) {
  return request({
    url: `/api/problems/${id}`,
    method: 'put',
    data
  })
}

// 删除题目
export function deleteProblem(id) {
  return request({
    url: `/api/problems/${id}`,
    method: 'delete'
  })
}

// 获取题目的测试用例
export function getTestCases(problemId) {
  return request({
    url: `/api/problems/${problemId}/testcases`,
    method: 'get'
  })
}

// 添加测试用例
export function addTestCase(problemId, data) {
  return request({
    url: `/api/problems/${problemId}/testcases`,
    method: 'post',
    data
  })
}

// 更新测试用例
export function updateTestCase(id, data) {
  return request({
    url: `/api/testcases/${id}`,
    method: 'put',
    data
  })
}

// 删除测试用例
export function deleteTestCase(id) {
  return request({
    url: `/api/testcases/${id}`,
    method: 'delete'
  })
}

// 提交代码
export function submitCode(data) {
  return request({
    url: '/api/submissions',
    method: 'post',
    data
  })
}

// 获取用户的提交记录
export function getUserSubmissions(params) {
  return request({
    url: '/api/submissions',
    method: 'get',
    params
  })
}

// 获取特定题目的用户提交记录
export function getUserProblemSubmissions(problemId, params) {
  return request({
    url: `/api/problems/${problemId}/submissions`,
    method: 'get',
    params
  })
}

// 获取用户题目状态
export function getUserProblemStatus() {
  return request({
    url: '/api/user/problem-status',
    method: 'get'
  })
} 