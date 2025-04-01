import request from '@/utils/request'

/**
 * 提交代码
 * @param {Object} data 包含problem_id, code, language
 * @returns {Promise}
 */
export function submitCode(data) {
  return request({
    url: '/api/submissions',
    method: 'post',
    data
  })
}

/**
 * 获取提交记录列表
 * @param {Object} params 查询参数：page, limit, problem_id, user_id, username, result, language, start_time, end_time
 * @returns {Promise}
 */
export function getSubmissions(params) {
  return request({
    url: '/api/submissions',
    method: 'get',
    params
  })
}

/**
 * 获取单个题目的提交记录
 * @param {Number} id 题目ID
 * @param {Object} params 查询参数：page, limit
 * @returns {Promise}
 */
export function getProblemSubmissions(id, params) {
  return request({
    url: `/api/problems/${id}/submissions`,
    method: 'get',
    params
  })
}

/**
 * 获取单个提交详情
 * @param {Number} id 提交ID
 * @returns {Promise}
 */
export function getSubmissionDetail(id) {
  return request({
    url: `/api/submissions/${id}`,
    method: 'get'
  })
}

/**
 * 重新提交代码
 * @param {Number} id 提交ID
 * @returns {Promise}
 */
export function resubmitCode(id) {
  return request({
    url: `/api/submissions/${id}/rejudge`,
    method: 'post'
  })
} 