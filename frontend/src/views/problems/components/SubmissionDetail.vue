<template>
  <div class="submission-detail-section">
    <div class="detail-header">
      <h4>提交详情 #{{ submissionId }}</h4>
      <el-button size="small" icon="el-icon-close" @click="closeDetail"></el-button>
    </div>
    <div v-if="submissionDetail" class="detail-content">
      <div class="detail-row">
        <span class="detail-label">ID：</span>
        <span>{{ submissionDetail.id }}</span>
      </div>
      <div class="detail-row">
        <span class="detail-label">状态：</span>
        <el-tag :type="getSubmissionStatusType(getStatusText(submissionDetail.result))" size="small" effect="light">
          {{ getStatusText(submissionDetail.result) }}
        </el-tag>
      </div>
      <div class="detail-row">
        <span class="detail-label">语言：</span>
        <span>C++</span>
      </div>
      <div class="detail-row">
        <span class="detail-label">运行时间：</span>
        <span>{{ submissionDetail.time_used > 0 ? submissionDetail.time_used + ' ms' : '-' }}</span>
      </div>
      <div class="detail-row">
        <span class="detail-label">内存占用：</span>
        <span>{{ submissionDetail.memory_used > 0 ? submissionDetail.memory_used + ' KB' : '-' }}</span>
      </div>
      <div class="detail-row">
        <span class="detail-label">提交时间：</span>
        <span>{{ formatDate(submissionDetail.created_at) }}</span>
      </div>
      
      <div class="code-section" v-if="submissionDetail.source_code">
        <div class="code-header">
          <span class="code-title">提交的代码</span>
          <el-button size="mini" type="primary" icon="el-icon-copy-document" @click="copyCode">复制代码</el-button>
        </div>
        <pre class="code-content">{{ submissionDetail.source_code }}</pre>
      </div>
      
      <div v-if="submissionDetail.error_message" class="error-section">
        <div class="error-header">错误信息</div>
        <pre class="error-content">{{ submissionDetail.error_message }}</pre>
      </div>
      
      <div class="detail-actions">
        <el-button type="primary" size="small" @click="resubmit" 
          :disabled="submissionDetail.result === 1">重新提交</el-button>
      </div>
    </div>
    <div v-else class="detail-loading">
      <el-skeleton :rows="6" animated />
    </div>
  </div>
</template>

<script>
export default {
  name: 'SubmissionDetail',
  props: {
    submissionId: {
      type: [Number, String],
      required: true
    },
    submissionDetail: {
      type: Object,
      default: null
    }
  },
  methods: {
    closeDetail() {
      this.$emit('close');
    },
    resubmit() {
      this.$emit('resubmit', this.submissionDetail);
    },
    copyCode() {
      if (this.submissionDetail && this.submissionDetail.source_code) {
        const textArea = document.createElement('textarea');
        textArea.value = this.submissionDetail.source_code;
        document.body.appendChild(textArea);
        textArea.select();
        document.execCommand('copy');
        document.body.removeChild(textArea);
        this.$message.success('代码已复制到剪贴板');
      }
    },
    // 根据提交状态返回不同的标签类型
    getSubmissionStatusType(status) {
      switch (status) {
        case '通过':
          return 'success';
        case '编译错误':
        case '运行错误':
          return 'danger';
        case '超时':
        case '内存超限':
          return 'warning';
        default:
          return 'info';
      }
    },
    // 获取评测结果文本表示
    getStatusText(result) {
      if (result === null || result === undefined) {
        return '未知';
      }
      
      switch (parseInt(result)) {
        case 0: return '待评测';
        case 1: return '评测中';
        case 2: return '通过';
        case 3: return '答案错误';
        case 4: return '超时';
        case 5: return '内存超限';
        case 6: return '运行错误';
        case 7: return '编译错误';
        case 8: return '系统错误';
        default: return '未知';
      }
    },
    // 格式化日期
    formatDate(timestamp) {
      if (!timestamp) return '';
      
      const date = new Date(timestamp * 1000);
      return date.toLocaleString();
    }
  }
}
</script>

<style lang="scss" scoped>
.submission-detail-section {
  border: 1px solid #dcdfe6;
  border-radius: 4px;
  margin-bottom: 20px;
  padding: 15px;
  background-color: #f8f8f9;
  
  .detail-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 15px;
    padding-bottom: 10px;
    border-bottom: 1px solid #ebeef5;
    
    h4 {
      margin: 0;
      color: #303133;
    }
  }
  
  .detail-content {
    .detail-row {
      margin-bottom: 10px;
      display: flex;
      align-items: center;
      
      .detail-label {
        font-weight: bold;
        margin-right: 10px;
        width: 80px;
      }
    }
    
    .code-section {
      margin-top: 20px;
      border: 1px solid #dcdfe6;
      border-radius: 4px;
      
      .code-header {
        background-color: #f5f7fa;
        padding: 10px;
        border-bottom: 1px solid #dcdfe6;
        display: flex;
        justify-content: space-between;
        align-items: center;
        
        .code-title {
          font-weight: bold;
        }
      }
      
      .code-content {
        padding: 15px;
        background-color: #fafafa;
        white-space: pre-wrap;
        font-family: "Courier New", Courier, monospace;
        font-size: 14px;
        overflow-x: auto;
        margin: 0;
      }
    }
    
    .error-section {
      margin-top: 20px;
      border: 1px solid #f56c6c;
      border-radius: 4px;
      
      .error-header {
        background-color: #fef0f0;
        padding: 10px;
        border-bottom: 1px solid #f56c6c;
        font-weight: bold;
        color: #f56c6c;
      }
      
      .error-content {
        padding: 15px;
        background-color: #fff;
        white-space: pre-wrap;
        font-family: "Courier New", Courier, monospace;
        font-size: 14px;
        color: #f56c6c;
        overflow-x: auto;
        margin: 0;
      }
    }
    
    .detail-actions {
      margin-top: 20px;
      text-align: right;
    }
  }
  
  .detail-loading {
    padding: 15px;
  }
}
</style> 