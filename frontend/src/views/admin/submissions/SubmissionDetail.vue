<template>
  <div class="submission-detail-container">
    <div class="page-header">
      <h1>提交详情</h1>
      <el-button type="primary" icon="el-icon-back" @click="goBack">返回列表</el-button>
    </div>
    
    <el-card v-loading="loading">
      <div v-if="submissionDetail" class="detail-content">
        <div class="detail-header">
          <div class="detail-header-item">
            <span class="label">提交ID：</span>
            <span class="value">{{ submissionDetail.id }}</span>
          </div>
          <div class="detail-header-item">
            <span class="label">题目：</span>
            <el-link type="primary" @click="navigateToProblem(submissionDetail.problem_id)">
              {{ submissionDetail.problem_id }}. {{ submissionDetail.problem_title || '未知题目' }}
            </el-link>
          </div>
          <div class="detail-header-item">
            <span class="label">用户：</span>
            <span class="value">{{ submissionDetail.username || '未知用户' }}</span>
          </div>
        </div>
        
        <el-divider></el-divider>
        
        <div class="detail-info">
          <el-row :gutter="20">
            <el-col :xs="24" :sm="8">
              <div class="info-item">
                <span class="label">评测结果：</span>
                <el-tag :type="getStatusTagType(submissionDetail.result)">
                  {{ formatResult(submissionDetail.result) }}
                </el-tag>
              </div>
            </el-col>
            <el-col :xs="24" :sm="8">
              <div class="info-item">
                <span class="label">得分：</span>
                <span class="value">{{ submissionDetail.score || 0 }}分</span>
              </div>
            </el-col>
            <el-col :xs="24" :sm="8">
              <div class="info-item">
                <span class="label">提交时间：</span>
                <span class="value">{{ formatDateTime(submissionDetail.created_at) }}</span>
              </div>
            </el-col>
            <el-col :xs="24" :sm="8">
              <div class="info-item">
                <span class="label">语言：</span>
                <el-tag size="medium" :type="getLanguageTagType(submissionDetail.language)">
                  {{ formatLanguage(submissionDetail.language) }}
                </el-tag>
              </div>
            </el-col>
            <el-col :xs="24" :sm="8">
              <div class="info-item">
                <span class="label">执行耗时：</span>
                <span class="value">{{ submissionDetail.time_used || 0 }} ms</span>
              </div>
            </el-col>
            <el-col :xs="24" :sm="8">
              <div class="info-item">
                <span class="label">内存消耗：</span>
                <span class="value">{{ formatMemory(submissionDetail.memory_used) }}</span>
              </div>
            </el-col>
          </el-row>
        </div>
        
        <el-divider></el-divider>
        
        <div class="code-section">
          <div class="section-header">
            <h3>提交代码</h3>
            <el-button size="small" type="primary" icon="el-icon-document-copy" @click="copyCode">
              复制代码
            </el-button>
          </div>
          
          <div class="code-wrapper">
            <pre><code>{{ submissionDetail.source_code }}</code></pre>
          </div>
        </div>
        
        <div v-if="submissionDetail.compile_error || submissionDetail.error_message" class="error-section">
          <div class="section-header">
            <h3>错误信息</h3>
          </div>
          
          <div class="error-wrapper">
            <pre>{{ submissionDetail.compile_error || submissionDetail.error_message }}</pre>
          </div>
        </div>
        
        <div v-if="submissionDetail.test_results && submissionDetail.test_results.length > 0" class="test-results-section">
          <div class="section-header">
            <h3>测试点结果</h3>
          </div>
          
          <el-table :data="submissionDetail.test_results" border stripe style="width: 100%">
            <el-table-column prop="id" label="测试点" width="80"></el-table-column>
            <el-table-column label="状态" width="120">
              <template slot-scope="scope">
                <el-tag :type="getStatusTagType(scope.row.result)">
                  {{ formatResult(scope.row.result) }}
                </el-tag>
              </template>
            </el-table-column>
            <el-table-column prop="score" label="得分" width="80"></el-table-column>
            <el-table-column prop="time_used" label="耗时" width="100">
              <template slot-scope="scope">
                {{ scope.row.time_used || 0 }} ms
              </template>
            </el-table-column>
            <el-table-column prop="memory_used" label="内存" width="120">
              <template slot-scope="scope">
                {{ formatMemory(scope.row.memory_used) }}
              </template>
            </el-table-column>
            <el-table-column prop="error_message" label="错误信息">
              <template slot-scope="scope">
                <el-popover
                  v-if="scope.row.error_message"
                  placement="top"
                  width="400"
                  trigger="hover">
                  <div style="max-height: 300px; overflow-y: auto;">
                    <pre>{{ scope.row.error_message }}</pre>
                  </div>
                  <el-button slot="reference" size="mini" type="danger" plain>查看错误</el-button>
                </el-popover>
                <span v-else>-</span>
              </template>
            </el-table-column>
          </el-table>
        </div>
        
        <div class="action-section">
          <el-button 
            type="primary" 
            @click="resubmit" 
            :disabled="submissionDetail.result === 1 || resubmitting"
            :loading="resubmitting">
            重新评测
          </el-button>
        </div>
      </div>
      
      <div v-else-if="!loading" class="no-data">
        <el-empty description="未找到提交记录"></el-empty>
      </div>
    </el-card>
  </div>
</template>

<script>
import { getSubmissionDetail, resubmitCode } from '@/api/submission'
import { formatDate, formatDateTime } from '@/utils/formatter'

export default {
  name: 'AdminSubmissionDetail',
  data() {
    return {
      submissionId: this.$route.params.id,
      submissionDetail: null,
      loading: false,
      resubmitting: false
    }
  },
  created() {
    this.fetchSubmissionDetail()
  },
  methods: {
    async fetchSubmissionDetail() {
      this.loading = true
      try {
        const response = await getSubmissionDetail(this.submissionId)
        this.submissionDetail = response.submission
        document.title = `提交详情 #${this.submissionId} - 管理后台`
      } catch (error) {
        console.error('获取提交详情失败:', error)
        this.$message.error('获取提交详情失败，请稍后重试')
      } finally {
        this.loading = false
      }
    },
    
    goBack() {
      this.$router.push('/admin/submissions')
    },
    
    navigateToProblem(problemId) {
      this.$router.push(`/admin/problems/edit/${problemId}`)
    },
    
    async resubmit() {
      this.resubmitting = true
      try {
        await resubmitCode(this.submissionId)
        this.$message.success('重新评测请求已提交')
        // 等待一段时间后刷新数据
        setTimeout(() => {
          this.fetchSubmissionDetail()
        }, 1000)
      } catch (error) {
        console.error('重新评测失败:', error)
        this.$message.error('重新评测失败，请稍后重试')
      } finally {
        this.resubmitting = false
      }
    },
    
    copyCode() {
      if (this.submissionDetail && this.submissionDetail.source_code) {
        const textArea = document.createElement('textarea')
        textArea.value = this.submissionDetail.source_code
        document.body.appendChild(textArea)
        textArea.select()
        document.execCommand('copy')
        document.body.removeChild(textArea)
        this.$message.success('代码已复制到剪贴板')
      }
    },
    
    formatDate(timestamp) {
      return formatDate(timestamp)
    },
    
    formatDateTime(timestamp) {
      return formatDateTime(timestamp)
    },
    
    formatResult(result) {
      const resultMap = {
        0: '等待评测',
        1: '评测中',
        2: '通过',
        3: '答案错误',
        4: '超时',
        5: '内存超限',
        6: '运行错误',
        7: '编译错误',
        8: '系统错误'
      }
      return resultMap[result] || '未知状态'
    },
    
    formatLanguage(language) {
      // 处理不同的语言格式（可能是数字索引或字符串）
      if (typeof language === 'number') {
        const languages = ['C', 'C++', 'Java', 'Python', 'JavaScript']
        return languages[language] || '未知语言'
      }
      const languageMap = {
        'cpp': 'C++',
        'c': 'C',
        'java': 'Java',
        'python': 'Python',
        'javascript': 'JavaScript'
      }
      return languageMap[language] || language || '未知语言'
    },
    
    formatMemory(memory) {
      if (!memory) return '0 KB'
      if (memory < 1024) return `${memory} KB`
      return `${(memory / 1024).toFixed(2)} MB`
    },
    
    getStatusTagType(result) {
      const types = {
        0: 'info',    // 等待评测
        1: 'warning', // 评测中
        2: 'success', // 通过
        3: 'danger',  // 答案错误
        4: 'danger',  // 超时
        5: 'danger',  // 内存超限
        6: 'danger',  // 运行错误
        7: 'danger',  // 编译错误
        8: 'danger'   // 系统错误
      }
      return types[result] || 'info'
    },
    
    getLanguageTagType(language) {
      // 为不同语言分配不同颜色标签
      const languageNumber = typeof language === 'number' ? language : -1
      const types = {
        0: 'info',      // C
        1: 'primary',   // C++
        2: 'success',   // Java
        3: 'warning',   // Python
        4: 'danger'     // JavaScript
      }
      return types[languageNumber] || 'info'
    }
  }
}
</script>

<style lang="scss" scoped>
.submission-detail-container {
  padding: 20px;
  
  .page-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 20px;
    
    h1 {
      margin: 0;
      font-size: 24px;
    }
  }
  
  .detail-content {
    .detail-header {
      display: flex;
      flex-wrap: wrap;
      gap: 20px;
      margin-bottom: 20px;
      
      .detail-header-item {
        .label {
          font-weight: bold;
          margin-right: 8px;
        }
      }
    }
    
    .detail-info {
      margin-bottom: 20px;
      
      .info-item {
        margin-bottom: 10px;
        
        .label {
          font-weight: bold;
          margin-right: 8px;
        }
      }
    }
    
    .section-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 15px;
      
      h3 {
        margin: 0;
        font-size: 18px;
      }
    }
    
    .code-section, .error-section, .test-results-section {
      margin-bottom: 30px;
    }
    
    .code-wrapper, .error-wrapper {
      background-color: #f5f7fa;
      border: 1px solid #e4e7ed;
      border-radius: 4px;
      padding: 15px;
      overflow-x: auto;
      
      pre {
        margin: 0;
        font-family: 'Courier New', Courier, monospace;
        font-size: 14px;
        white-space: pre-wrap;
      }
    }
    
    .error-wrapper {
      background-color: #fff0f0;
      border-color: #ffb8b8;
      
      pre {
        color: #f56c6c;
      }
    }
    
    .action-section {
      margin-top: 30px;
      text-align: right;
    }
  }
  
  .no-data {
    padding: 40px 0;
  }
}

@media (max-width: 767px) {
  .submission-detail-container {
    padding: 10px;
    
    .detail-header {
      flex-direction: column;
      gap: 10px;
    }
  }
}
</style> 