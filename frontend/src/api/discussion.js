import request from '@/utils/request';

/**
 * 获取所有讨论
 * @param {Object} params 查询参数
 * @returns {Promise} 返回请求Promise对象
 */
export function getAllDiscussions(params) {
  return request({
    url: '/api/discussions',
    method: 'get',
    params
  });
}

/**
 * 获取题目相关讨论
 * @param {Number} problemId 题目ID
 * @param {Object} params 查询参数
 * @returns {Promise} 返回请求Promise对象
 */
export function getProblemDiscussions(problemId, params) {
  return request({
    url: `/api/problems/${problemId}/discussions`,
    method: 'get',
    params
  });
}

/**
 * 获取讨论详情
 * @param {Number} discussionId 讨论ID
 * @returns {Promise} 返回请求Promise对象
 */
export function getDiscussionDetail(discussionId) {
  return request({
    url: `/api/discussions/${discussionId}`,
    method: 'get'
  });
}

/**
 * 创建讨论
 * @param {Object} data 讨论数据
 * @returns {Promise} 返回请求Promise对象
 */
export function createDiscussion(data) {
  return request({
    url: '/api/discussions',
    method: 'post',
    data
  });
}

/**
 * 更新讨论
 * @param {Number} discussionId 讨论ID
 * @param {Object} data 讨论数据
 * @returns {Promise} 返回请求Promise对象
 */
export function updateDiscussion(discussionId, data) {
  return request({
    url: `/api/discussions/${discussionId}`,
    method: 'put',
    data
  });
}

/**
 * 删除讨论
 * @param {Number} discussionId 讨论ID
 * @returns {Promise} 返回请求Promise对象
 */
export function deleteDiscussion(discussionId) {
  return request({
    url: `/api/discussions/${discussionId}`,
    method: 'delete'
  });
}

/**
 * 获取讨论回复
 * @param {Number} discussionId 讨论ID
 * @param {Object} params 查询参数
 * @returns {Promise} 返回请求Promise对象
 */
export function getDiscussionReplies(discussionId, params) {
  return request({
    url: `/api/discussions/${discussionId}/replies`,
    method: 'get',
    params
  });
}

/**
 * 创建回复
 * @param {Number} discussionId 讨论ID
 * @param {Object} data 回复数据
 * @returns {Promise} 返回请求Promise对象
 */
export function createDiscussionReply(discussionId, data) {
  return request({
    url: `/api/discussions/${discussionId}/replies`,
    method: 'post',
    data
  });
}

/**
 * 删除回复
 * @param {Number} replyId 回复ID
 * @returns {Promise} 返回请求Promise对象
 */
export function deleteDiscussionReply(replyId) {
  return request({
    url: `/api/replies/${replyId}`,
    method: 'delete'
  });
} 