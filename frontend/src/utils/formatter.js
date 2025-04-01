/**
 * 格式化日期时间
 * @param {Number|String} timestamp 时间戳（毫秒）或日期字符串
 * @param {Boolean} showSeconds 是否显示秒
 * @returns {String} 格式化后的日期时间字符串（YYYY-MM-DD HH:mm:ss）
 */
export function formatDateTime(timestamp, showSeconds = true) {
  if (!timestamp) return '';
  
  let date;
  if (typeof timestamp === 'string') {
    // 尝试将字符串转换为Date对象
    date = new Date(timestamp);
    if (isNaN(date.getTime())) {
      // 如果是无效日期，尝试将其作为时间戳处理
      date = new Date(parseInt(timestamp));
    }
  } else {
    // 处理时间戳（毫秒）
    date = new Date(timestamp);
  }
  
  // 如果转换后仍然是无效日期，则返回空字符串
  if (isNaN(date.getTime())) return '';
  
  const year = date.getFullYear();
  const month = String(date.getMonth() + 1).padStart(2, '0');
  const day = String(date.getDate()).padStart(2, '0');
  const hours = String(date.getHours()).padStart(2, '0');
  const minutes = String(date.getMinutes()).padStart(2, '0');
  
  if (showSeconds) {
    const seconds = String(date.getSeconds()).padStart(2, '0');
    return `${year}-${month}-${day} ${hours}:${minutes}:${seconds}`;
  } else {
    return `${year}-${month}-${day} ${hours}:${minutes}`;
  }
}

/**
 * 格式化日期（不包含时间）
 * @param {Number|String} timestamp 时间戳（毫秒）或日期字符串
 * @returns {String} 格式化后的日期字符串（YYYY-MM-DD）
 */
export function formatDate(timestamp) {
  if (!timestamp) return '';
  
  let date;
  if (typeof timestamp === 'string') {
    // 尝试将字符串转换为Date对象
    date = new Date(timestamp);
    if (isNaN(date.getTime())) {
      // 如果是无效日期，尝试将其作为时间戳处理
      date = new Date(parseInt(timestamp));
    }
  } else {
    // 处理时间戳（毫秒）
    date = new Date(timestamp);
  }
  
  // 如果转换后仍然是无效日期，则返回空字符串
  if (isNaN(date.getTime())) return '';
  
  const year = date.getFullYear();
  const month = String(date.getMonth() + 1).padStart(2, '0');
  const day = String(date.getDate()).padStart(2, '0');
  
  return `${year}-${month}-${day}`;
}

/**
 * 格式化时间间隔（将秒数转换为人类可读的时间格式）
 * @param {Number} seconds 秒数
 * @returns {String} 格式化后的时间间隔（例如：2小时30分钟）
 */
export function formatTimeInterval(seconds) {
  if (!seconds || seconds < 0) return '0秒';
  
  const days = Math.floor(seconds / (24 * 3600));
  const hours = Math.floor((seconds % (24 * 3600)) / 3600);
  const minutes = Math.floor((seconds % 3600) / 60);
  const remainingSeconds = Math.floor(seconds % 60);
  
  let result = '';
  if (days > 0) result += `${days}天`;
  if (hours > 0) result += `${hours}小时`;
  if (minutes > 0) result += `${minutes}分钟`;
  if (remainingSeconds > 0 && !days && hours < 10) result += `${remainingSeconds}秒`;
  
  return result || '0秒';
} 