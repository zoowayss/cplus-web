<template>
  <div class="submission-list-container">
    <el-card class="submission-list-card">
      <div slot="header" class="card-header" v-if="showFilter">
        <span>我的提交记录</span>
        <div class="card-header-buttons">
          <el-button size="small" icon="el-icon-refresh" @click="fetchSubmissions">刷新</el-button>
        </div>
      </div>
      
      <!-- 筛选条件 -->
      <div class="filter-bar" v-if="showFilter">
        <el-form :inline="true" :model="filterForm" class="form-inline">
          <el-form-item label="题目ID">
            <el-input v-model="filterForm.problemId" placeholder="输入题目ID" clearable></el-input>
          </el-form-item>
          <el-form-item label="状态">
            <el-select v-model="filterForm.status" placeholder="选择状态" clearable>
              <el-option v-for="item in statusOptions" :key="item.value" :label="item.label" :value="item.value"></el-option>
            </el-select>
          </el-form-item>
          <el-form-item label="语言">
            <el-select v-model="filterForm.language" placeholder="选择语言" clearable>
              <el-option v-for="item in languageOptions" :key="item.value" :label="item.label" :value="item.value"></el-option>
            </el-select>
          </el-form-item>
          <el-form-item>
            <el-button type="primary" @click="handleFilter">查询</el-button>
            <el-button @click="resetFilter">重置</el-button>
          </el-form-item>
        </el-form>
      </div>
      
      <!-- 提交记录表格 -->
      <el-table
        :data="submissions"
        border
        style="width: 100%"
        v-loading="loading"
      >
        <el-table-column prop="id" label="ID" width="80" />
        <el-table-column prop="problem_id" label="题目ID" width="80" />
        <el-table-column prop="problem_title" label="题目标题" min-width="180" />
        <el-table-column prop="language" label="语言" width="100" />
        <el-table-column label="状态" width="120">
          <template slot-scope="scope">
            <el-tag :type="getSubmissionStatusType(getStatusText(scope.row.result))" size="small" effect="light">
              {{ getStatusText(scope.row.result) }}
            </el-tag>
          </template>
        </el-table-column>
        <el-table-column label="运行时间" width="110">
          <template slot-scope="scope">
            <span>{{ scope.row.time_used > 0 ? scope.row.time_used + ' ms' : '-' }}</span>
          </template>
        </el-table-column>
        <el-table-column label="内存占用" width="110">
          <template slot-scope="scope">
            <span>{{ scope.row.memory_used > 0 ? scope.row.memory_used + ' KB' : '-' }}</span>
          </template>
        </el-table-column>
        <el-table-column label="提交时间" width="180">
          <template slot-scope="scope">
            <span>{{ formatDate(scope.row.created_at) }}</span>
          </template>
        </el-table-column>
        <el-table-column label="操作" width="160">
          <template slot-scope="scope">
            <el-button type="text" size="small" @click="viewSubmission(scope.row)">查看详情</el-button>
            <el-button 
              type="text" 
              size="small" 
              @click="resubmit(scope.row)"
              :disabled="scope.row.result === 1"
            >重新提交</el-button>
          </template>
        </el-table-column>
      </el-table>
      
      <!-- 分页 -->
      <div class="pagination-container">
        <el-pagination
          background
          @size-change="handleSizeChange"
          @current-change="handleCurrentChange"
          :current-page="currentPage"
          :page-sizes="[10, 20, 50, 100]"
          :page-size="pageSize"
          layout="total, sizes, prev, pager, next, jumper"
          :total="total"
        />
      </div>
      
      <!-- 提交详情对话框 -->
      <el-dialog
        title="提交详情"
        :visible.sync="submissionDialogVisible"
        width="60%"
        class="submission-dialog"
      >
        <div v-if="currentSubmission" class="submission-detail">
          <div class="detail-row">
            <span class="detail-label">ID：</span>
            <span>{{ currentSubmission.id }}</span>
          </div>
          <div class="detail-row" v-if="currentSubmission.problem_title">
            <span class="detail-label">题目：</span>
            <span>{{ currentSubmission.problem_title }}</span>
          </div>
          <div class="detail-row" v-else>
            <span class="detail-label">题目ID：</span>
            <span>{{ currentSubmission.problem_id }}</span>
          </div>
          <div class="detail-row">
            <span class="detail-label">状态：</span>
            <el-tag :type="getSubmissionStatusType(getStatusText(currentSubmission.result))" size="small" effect="light">
              {{ getStatusText(currentSubmission.result) }}
            </el-tag>
          </div>
          <div class="detail-row">
            <span class="detail-label">语言：</span>
            <span>{{ currentSubmission.language }}</span>
          </div>
          <div class="detail-row">
            <span class="detail-label">运行时间：</span>
            <span>{{ currentSubmission.time_used > 0 ? currentSubmission.time_used + ' ms' : '-' }}</span>
          </div>
          <div class="detail-row">
            <span class="detail-label">内存占用：</span>
            <span>{{ currentSubmission.memory_used > 0 ? currentSubmission.memory_used + ' KB' : '-' }}</span>
          </div>
          <div class="detail-row">
            <span class="detail-label">提交时间：</span>
            <span>{{ formatDate(currentSubmission.created_at) }}</span>
          </div>
          
          <div class="code-section" v-if="currentSubmission.source_code">
            <div class="code-header">
              <span class="code-title">提交的代码</span>
              <el-button size="mini" type="primary" icon="el-icon-copy-document" @click="copyCode">复制代码</el-button>
            </div>
            <pre class="code-content">{{ currentSubmission.source_code }}</pre>
          </div>
          
          <div v-if="currentSubmission.error_message" class="error-section">
            <div class="error-header">错误信息</div>
            <pre class="error-content">{{ currentSubmission.error_message }}</pre>
          </div>
        </div>
        <span slot="footer" class="dialog-footer">
          <el-button @click="submissionDialogVisible = false">关闭</el-button>
          <el-button 
            type="primary" 
            @click="resubmit(currentSubmission)" 
            :disabled="currentSubmission && currentSubmission.result === 1"
          >重新提交</el-button>
        </span>
      </el-dialog>
    </el-card>
  </div>
</template>

<script>
import axios from 'axios';
import { getUserSubmissions, getUserProblemSubmissions } from '@/api/problem';

export default {
  name: 'SubmissionList',
  props: {
    params: {
      type: Object,
      default: () => ({})
    },
    showFilter: {
      type: Boolean,
      default: true
    },
    detailMode: {
      type: String,
      default: 'dialog' // 可选值：'dialog'(弹窗模式), 'inline'(内联模式)
    }
  },
  data() {
    return {
      loading: false,
      submissions: [],
      total: 0,
      currentPage: 1,
      pageSize: 10,
      filterForm: {
        problemId: '',
        status: '',
        language: ''
      },
      statusOptions: [
        { label: '待评测', value: 0 },
        { label: '评测中', value: 1 },
        { label: '通过', value: 2 },
        { label: '答案错误', value: 3 },
        { label: '超时', value: 4 },
        { label: '内存超限', value: 5 },
        { label: '运行错误', value: 6 },
        { label: '编译错误', value: 7 },
        { label: '系统错误', value: 8 }
      ],
      languageOptions: [
        { label: 'C', value: 'c' },
        { label: 'C++', value: 'cpp' },
        { label: 'Java', value: 'java' },
        { label: 'Python', value: 'python' },
        { label: 'JavaScript', value: 'javascript' }
      ],
      submissionDialogVisible: false,
      currentSubmission: null,
    };
  },
  created() {
    // 检查用户是否已登录
    if (!this.$store.getters.isAuthenticated) {
      this.$message.warning('请先登录');
      this.$router.push('/login');
      return;
    }
    
    this.fetchSubmissions();
    
    // 如果传入了 submissionId，则显示该提交的详情
    if (this.params && this.params.submissionId) {
      this.fetchSubmissionDetail(this.params.submissionId);
    }
  },
  methods: {
    // 获取提交记录列表
    fetchSubmissions() {
      this.loading = true;
      
      // 构建API请求参数
      const params = {
        page: this.currentPage,
        limit: this.pageSize
      };
      
      if (this.filterForm.problemId) {
        params.problem_id = this.filterForm.problemId;
      }
      
      if (this.filterForm.status !== '') {
        params.result = this.filterForm.status;
      }
      
      if (this.filterForm.language) {
        params.language = this.filterForm.language;
      }

      // 判断使用哪个API来获取提交记录
      if (this.$route.params.id) {
        // 获取特定题目的提交记录
        const problemId = this.$route.params.id;
        getUserProblemSubmissions(problemId, params)
          .then(response => {
            if (response.status === 'ok') {
              this.submissions = response.submissions || [];
              this.total = response.total || 0;
              this.currentPage = response.page || 1;
              this.pageSize = response.limit || 10;
            } else {
              this.$message.error(response.message || '获取提交记录失败');
            }
          })
          .catch(error => {
            console.error('获取提交记录失败:', error);
            this.$message.error('获取提交记录失败，请稍后再试');
          })
          .finally(() => {
            this.loading = false;
          });
      } else {
        // 获取用户的所有提交记录
        getUserSubmissions(params)
          .then(response => {
            if (response.status === 'ok') {
              this.submissions = response.submissions || [];
              this.total = response.total || 0;
              this.currentPage = response.page || 1;
              this.pageSize = response.limit || 10;
            } else {
              this.$message.error(response.message || '获取提交记录失败');
            }
          })
          .catch(error => {
            console.error('获取提交记录失败:', error);
            this.$message.error('获取提交记录失败，请稍后再试');
          })
          .finally(() => {
            this.loading = false;
          });
      }
    },
    
    // 获取单个提交详情
    fetchSubmissionDetail(submissionId) {
      import('@/utils/request').then(module => {
        const request = module.default;
        request({
          url: `/api/submissions/${submissionId}`,
          method: 'get'
        })
          .then(response => {
            if (response.status === 'ok') {
              this.currentSubmission = response.submission;
              this.submissionDialogVisible = true;
            } else {
              this.$message.error(response.message || '获取提交详情失败');
            }
          })
          .catch(error => {
            console.error('获取提交详情失败:', error);
            this.$message.error('获取提交详情失败，请稍后再试');
          });
      });
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
    
    // 查看提交详情
    viewSubmission(submission) {
      // 先设置基本信息
      this.currentSubmission = submission;
      
      // 根据详情模式决定显示方式
      if (this.detailMode === 'inline') {
        // 触发事件通知父组件，让父组件处理详情显示
        this.$emit('view-submission', submission.id);
      } else {
        // 弹窗模式
        this.submissionDialogVisible = true;
        
        // 如果需要获取完整的提交详情（包括源代码等）
        import('@/utils/request').then(module => {
          const request = module.default;
          request({
            url: `/api/submissions/${submission.id}`,
            method: 'get'
          })
            .then(response => {
              if (response.status === 'ok') {
                this.currentSubmission = response.submission;
              } else {
                this.$message.error(response.message || '获取提交详情失败');
              }
            })
            .catch(error => {
              console.error('获取提交详情失败:', error);
              this.$message.error('获取提交详情失败，请稍后再试');
            });
        });
      }
    },
    
    // 重新提交代码
    resubmit(submission) {
      this.$confirm('确定要重新提交这段代码吗?', '提示', {
        confirmButtonText: '确定',
        cancelButtonText: '取消',
        type: 'warning'
      }).then(() => {
        import('@/utils/request').then(module => {
          const request = module.default;
          request({
            url: `/api/submissions/resubmit/${submission.id}`,
            method: 'post'
          })
            .then(response => {
              if (response.status === 'ok') {
                this.$message.success('重新提交成功，正在评测');
                
                // 直接刷新数据，不在本地更新
                this.fetchSubmissions();
                
                // 如果当前有打开详情弹窗，也刷新详情数据
                if (this.submissionDialogVisible && this.currentSubmission && this.currentSubmission.id === submission.id) {
                  this.viewSubmission(submission);
                }
              } else {
                this.$message.error(response.message || '重新提交失败');
              }
            })
            .catch(error => {
              console.error('重新提交失败:', error);
              this.$message.error('重新提交失败，请稍后再试');
            });
        });
      }).catch(() => {
        // 用户取消操作
      });
    },
    
    // 复制代码
    copyCode() {
      if (this.currentSubmission && this.currentSubmission.source_code) {
        const textArea = document.createElement('textarea');
        textArea.value = this.currentSubmission.source_code;
        document.body.appendChild(textArea);
        textArea.select();
        document.execCommand('copy');
        document.body.removeChild(textArea);
        this.$message.success('代码已复制到剪贴板');
      }
    },
    
    // 筛选
    handleFilter() {
      this.currentPage = 1;
      this.fetchSubmissions();
    },
    
    // 重置筛选条件
    resetFilter() {
      this.filterForm = {
        problemId: '',
        status: '',
        language: ''
      };
      this.currentPage = 1;
      this.fetchSubmissions();
    },
    
    // 每页数量变化
    handleSizeChange(val) {
      this.pageSize = val;
      this.fetchSubmissions();
    },
    
    // 页码变化
    handleCurrentChange(val) {
      this.currentPage = val;
      this.fetchSubmissions();
    },
    
    // 根据提交状态返回不同的标签类型
    getSubmissionStatusType(status) {
      switch (status) {
        case '通过':
          return 'success';
        case '编译错误':
        case '运行错误':
        case '答案错误':
        case '系统错误':
          return 'danger';
        case '超时':
        case '内存超限':
          return 'warning';
        case '评测中':
          return 'info';
        case '待评测':
          return 'info';
        default:
          return 'info';
      }
    },
    
    // 格式化日期
    formatDate(timestamp) {
      if (!timestamp) return '';
      
      const date = new Date(timestamp * 1000);
      return date.toLocaleString();
    }
  }
};
</script>

<style lang="scss" scoped>
.submission-list-container {
  padding: 20px;
  
  .submission-list-card {
    .card-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
    
    .filter-bar {
      margin-bottom: 20px;
    }
    
    .pagination-container {
      margin-top: 20px;
      text-align: right;
    }
  }
  
  .submission-dialog {
    .submission-detail {
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
    }
  }
}

@media screen and (max-width: 768px) {
  .submission-list-container {
    padding: 10px;
    
    .form-inline {
      .el-form-item {
        margin-right: 0;
        width: 100%;
        
        .el-form-item__label {
          width: 60px;
        }
        
        .el-form-item__content {
          width: calc(100% - 60px);
        }
      }
    }
  }
}
</style> 