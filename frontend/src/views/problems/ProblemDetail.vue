<template>
  <div class="problem-detail-container">
    <el-card class="problem-detail-card" v-loading="loading" shadow="never">
      <div slot="header" class="card-header">
        <span v-if="problem.title">{{ problem.title }}</span>
        <div class="card-header-info">
          <el-tag :type="getDifficultyType(problem.difficulty)" size="small">{{ problem.difficulty }}</el-tag>
          <el-button size="small" icon="el-icon-back" @click="goBackToList">返回题目列表</el-button>
        </div>
      </div>
      
      <div v-if="problem.id">
        <!-- 题目信息 -->
        <el-tabs v-model="activeTab" type="card">
          <el-tab-pane label="题目描述" name="description">
            <div class="content-section">              
              <h3>题目描述</h3>
              <div class="markdown-content">{{ problem.description }}</div>
              
              <h3>输入格式</h3>
              <div class="markdown-content">{{ problem.input_format }}</div>
              
              <h3>输出格式</h3>
              <div class="markdown-content">{{ problem.output_format }}</div>
              
              <h3>示例</h3>
              <div class="example-section">
                <div class="example-input">
                  <div class="example-title">输入:</div>
                  <pre>{{ problem.example_input }}</pre>
                </div>
                <div class="example-output">
                  <div class="example-title">输出:</div>
                  <pre>{{ problem.example_output }}</pre>
                </div>
              </div>
              
              <div v-if="problem.hint">
                <h3>提示</h3>
                <div class="markdown-content">{{ problem.hint }}</div>
              </div>
            </div>
          </el-tab-pane>
          
          <el-tab-pane label="提交" name="submit">
            <div class="submit-section">
              <h3>代码编辑器</h3>
              
              <div class="language-selector">
                <span>选择语言：</span>
                <el-select v-model="language" placeholder="请选择编程语言">
                  <el-option label="C++" value="cpp" />
                  <el-option label="Java" value="java" />
                  <el-option label="Python" value="python" />
                </el-select>
              </div>
              
              <!-- 简化的代码编辑器 -->
              <div class="code-editor-wrapper">
                <el-input
                  type="textarea"
                  v-model="code"
                  :rows="20"
                  placeholder="在此输入您的代码"
                  class="code-editor"
                />
              </div>
              
              <!-- 提交按钮 -->
              <div class="submit-button-container">
                <el-button type="primary" @click="submitCode" :loading="submitting">提交代码</el-button>
              </div>
            </div>
          </el-tab-pane>
          
          <el-tab-pane label="提交记录" name="submissions">
            <div class="submissions-section">
              <h3>我的提交记录</h3>
              
              <div v-if="currentSubmissionId" class="submission-detail-section">
                <div class="detail-header">
                  <h4>提交详情 #{{ currentSubmissionId }}</h4>
                  <el-button size="small" icon="el-icon-close" @click="closeSubmissionDetail"></el-button>
                </div>
                <div v-if="currentSubmissionDetail" class="detail-content">
                  <div class="detail-row">
                    <span class="detail-label">ID：</span>
                    <span>{{ currentSubmissionDetail.id }}</span>
                  </div>
                  <div class="detail-row">
                    <span class="detail-label">状态：</span>
                    <el-tag :type="getSubmissionStatusType(getStatusText(currentSubmissionDetail.result))" size="small" effect="light">
                      {{ getStatusText(currentSubmissionDetail.result) }}
                    </el-tag>
                  </div>
                  <div class="detail-row">
                    <span class="detail-label">语言：</span>
                    <span>{{ currentSubmissionDetail.language }}</span>
                  </div>
                  <div class="detail-row">
                    <span class="detail-label">运行时间：</span>
                    <span>{{ currentSubmissionDetail.time_used > 0 ? currentSubmissionDetail.time_used + ' ms' : '-' }}</span>
                  </div>
                  <div class="detail-row">
                    <span class="detail-label">内存占用：</span>
                    <span>{{ currentSubmissionDetail.memory_used > 0 ? currentSubmissionDetail.memory_used + ' KB' : '-' }}</span>
                  </div>
                  <div class="detail-row">
                    <span class="detail-label">提交时间：</span>
                    <span>{{ formatDate(currentSubmissionDetail.created_at) }}</span>
                  </div>
                  
                  <div class="code-section" v-if="currentSubmissionDetail.source_code">
                    <div class="code-header">
                      <span class="code-title">提交的代码</span>
                      <el-button size="mini" type="primary" icon="el-icon-copy-document" @click="copySubmissionCode">复制代码</el-button>
                    </div>
                    <pre class="code-content">{{ currentSubmissionDetail.source_code }}</pre>
                  </div>
                  
                  <div v-if="currentSubmissionDetail.error_message" class="error-section">
                    <div class="error-header">错误信息</div>
                    <pre class="error-content">{{ currentSubmissionDetail.error_message }}</pre>
                  </div>
                  
                  <div class="detail-actions">
                    <el-button type="primary" size="small" @click="resubmitCode(currentSubmissionDetail)" 
                      :disabled="currentSubmissionDetail.result === 1">重新提交</el-button>
                  </div>
                </div>
                <div v-else class="detail-loading">
                  <el-skeleton :rows="6" animated />
                </div>
              </div>
              
              <!-- 使用SubmissionList组件，不显示筛选区域，使用内联详情模式 -->
              <SubmissionList 
                ref="submissionList"
                :params="{ problemId: problemId }" 
                :showFilter="false" 
                detailMode="inline"
                @view-submission="handleViewSubmission"
              />
            </div>
          </el-tab-pane>
          
          <!-- 添加讨论标签页 -->
          <el-tab-pane label="讨论" name="discussions">
            <div class="discussions-section">
              <DiscussionList :problemId="Number(problemId)" />
            </div>
          </el-tab-pane>
        </el-tabs>
      </div>
      
      <!-- 题目不存在的提示 -->
      <el-empty v-else-if="!loading" description="题目不存在或已被删除" />
    </el-card>
  </div>
</template>

<script>
import { getProblemDetail, submitCode, getUserProblemSubmissions } from '@/api/problem';
// 导入SubmissionList组件
import SubmissionList from '@/views/problems/SubmissionList.vue';
// 导入DiscussionList组件
import DiscussionList from '@/views/problems/DiscussionList.vue';
import request from '@/utils/request';

export default {
  name: 'ProblemDetail',
  components: {
    SubmissionList, // 注册组件
    DiscussionList  // 注册讨论组件
  },
  props: {
    params: {
      type: Object,
      default: () => ({})
    },
    isDialogMode: {
      type: Boolean,
      default: false
    }
  },
  data() {
    return {
      loading: false,
      submitting: false,
      loadingSubmissions: false,
      problem: {},
      problemId: null,
      activeTab: 'description',
      language: 'cpp',
      code: '',
      submissions: [],
      // 分页参数
      pagination: {
        page: 1,
        limit: 10,
        total: 0
      },
      // 当前查看的提交详情
      currentSubmissionId: null,
      currentSubmissionDetail: null
    };
  },
  created() {
    // 确定题目ID，优先从路由参数获取，其次从props获取
    this.problemId = this.$route.params.id || (this.params && this.params.problemId);
    console.log('ProblemDetail created, ID:', this.problemId, '参数:', this.params);
    
    if (this.problemId) {
      this.fetchProblemDetail();
      this.fetchUserSubmissions();
    } else {
      this.$message.error('未提供有效的题目ID');
    }
  },
  mounted() {
    console.log('ProblemDetail mounted, params:', this.params);
    
    // 监听参数变化
    this.$watch(
      'params.problemId',
      (newId) => {
        console.log('参数problemId变化:', newId);
        if (newId && newId !== this.problemId) {
          this.problemId = newId;
          this.fetchProblemDetail();
          this.fetchUserSubmissions();
        }
      }
    );
    
    // 处理导航守卫，监听路由变化
    this.$watch('$route.params.id', (newId) => {
      console.log('路由参数id变化:', newId);
      if (newId && newId !== this.problemId) {
        this.problemId = newId;
        this.fetchProblemDetail();
        this.fetchUserSubmissions();
      }
    });
  },
  methods: {
    // 获取题目详情
    fetchProblemDetail() {
      this.loading = true;
      getProblemDetail(this.problemId)
        .then(response => {
          if (response.status === 'ok' && response.problem) {
            this.problem = response.problem;
          } else {
            this.$message.error(response.message || '获取题目详情失败');
            // 题目不存在或已删除，返回列表页
            // this.$router.push('/problems');
          }
        })
        .catch(error => {
          console.error('获取题目详情失败:', error);
          this.$message.error('获取题目详情失败，请稍后再试');
        })
        .finally(() => {
          this.loading = false;
        });
    },
    
    // 获取用户提交记录
    fetchUserSubmissions() {
      if (!this.problemId) return;
      
      this.loadingSubmissions = true;
      
      const params = {
        page: this.pagination.page,
        limit: this.pagination.limit
      };
      
      getUserProblemSubmissions(this.problemId, params)
        .then(response => {
          console.log('获取用户提交记录响应:', response);
          if (response.status === 'ok') {
            // 处理提交记录数据
            if (response.submissions && Array.isArray(response.submissions)) {
              this.submissions = response.submissions;
            } else if (response.data && Array.isArray(response.data)) {
              this.submissions = response.data;
            } else if (response.result && Array.isArray(response.result)) {
              this.submissions = response.result;
            } else {
              this.submissions = [];
            }
            
            // 更新分页总数
            if (response.total) {
              this.pagination.total = response.total;
            }
          } else {
            this.$message.warning(response.message || '获取提交记录失败');
            this.submissions = [];
          }
        })
        .catch(error => {
          console.error('获取用户提交记录失败:', error);
          this.$message.warning('获取提交记录失败，请稍后再试');
          this.submissions = [];
        })
        .finally(() => {
          this.loadingSubmissions = false;
        });
    },
    
    // 提交代码
    submitCode() {
      if (!this.code.trim()) {
        this.$message.warning('请输入代码');
        return;
      }
      
      this.submitting = true;
      
      const data = {
        problem_id: this.problemId,
        code: this.code,
        language: this.language
      };
      
      submitCode(data)
        .then(response => {
          // 判断响应状态
          if (response.status === 'ok' || (response.message && response.message.includes('正在评测'))) {
            this.$message.success('代码提交成功，正在评测');
            
            // 切换到提交记录选项卡
            this.activeTab = 'submissions';
            
            // 直接刷新提交记录，获取最新评测结果
            this.fetchUserSubmissions();
          } else {
            // 真正的错误情况
            this.$message.error(response.message || '代码提交失败');
          }
        })
        .catch(error => {
          console.error('代码提交失败:', error);
          
          // 检查错误消息是否包含"正在评测"，如果包含则视为成功
          if (error.message && error.message.includes('正在评测')) {
            this.$message.success('代码提交成功，正在评测');
            
            // 切换到提交记录选项卡
            this.activeTab = 'submissions';
            
            // 直接刷新提交记录
            this.fetchUserSubmissions();
          } else {
            // 真正的错误
            this.$message.error('代码提交失败，请稍后再试');
          }
        })
        .finally(() => {
          this.submitting = false;
        });
    },
    
    // 根据难度返回不同的标签类型
    getDifficultyType(difficulty) {
      switch (difficulty) {
        case '简单':
          return 'success';
        case '中等':
          return 'warning';
        case '困难':
          return 'danger';
        default:
          return 'info';
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
          return 'warning';
        case 0:
          return '评测中';
        default:
          return 'info';
      }
    },
    
    // 获取语言显示名称
    getLanguageDisplay(language) {
      const languageMap = {
        cpp: 'C++',
        java: 'Java',
        python: 'Python'
      };
      return languageMap[language] || language;
    },
    
    // 格式化日期
    formatDate(timestamp) {
      if (!timestamp) return '';
      
      const date = new Date(timestamp * 1000);
      return date.toLocaleString();
    },
    
    // 返回题目列表
    goBackToList() {
      // 检查是否在用户主页或管理员主页环境中
      if (this.$parent && this.$parent.$options.name === 'UserHome') {
        // 在用户主页内，通过调用父组件方法切换到题目列表
        this.$parent.handleSelect('problems');
      } else if (this.$parent && this.$parent.$options.name === 'Home') {
        // 在管理员主页内，通过调用父组件方法切换到题目列表
        this.$parent.handleSelect('problem-list');
      } else {
        // 独立页面模式下，直接返回题目列表页面
        this.$router.push('/problems');
      }
    },
    
    // 处理分页大小变化
    handleSizeChange(newSize) {
      this.pagination.limit = newSize;
      this.fetchUserSubmissions();
    },
    
    // 处理当前页变化
    handleCurrentChange(newPage) {
      this.pagination.page = newPage;
      this.fetchUserSubmissions();
    },
    
    // 处理查看提交详情事件
    handleViewSubmission(submissionId) {
      this.currentSubmissionId = submissionId;
      this.currentSubmissionDetail = null;
      
      // 获取提交详情
      request({
        url: `/api/submissions/${submissionId}`,
        method: 'get'
      })
        .then(response => {
          if (response.status === 'ok') {
            this.currentSubmissionDetail = response.submission;
          } else {
            this.$message.error(response.message || '获取提交详情失败');
          }
        })
        .catch(error => {
          console.error('获取提交详情失败:', error);
          this.$message.error('获取提交详情失败，请稍后再试');
        });
    },
    
    // 关闭提交详情
    closeSubmissionDetail() {
      this.currentSubmissionId = null;
      this.currentSubmissionDetail = null;
    },
    
    // 复制提交代码
    copySubmissionCode() {
      if (this.currentSubmissionDetail && this.currentSubmissionDetail.source_code) {
        const textArea = document.createElement('textarea');
        textArea.value = this.currentSubmissionDetail.source_code;
        document.body.appendChild(textArea);
        textArea.select();
        document.execCommand('copy');
        document.body.removeChild(textArea);
        this.$message.success('代码已复制到剪贴板');
      }
    },
    
    // 重新提交代码
    resubmitCode(submission) {
      this.$confirm('确定要重新提交这段代码吗?', '提示', {
        confirmButtonText: '确定',
        cancelButtonText: '取消',
        type: 'warning'
      }).then(() => {
        request({
          url: `/api/submissions/resubmit/${submission.id}`,
          method: 'post'
        })
          .then(response => {
            if (response.status === 'ok') {
              this.$message.success('重新提交成功，正在评测');
              
              // 关闭详情
              this.closeSubmissionDetail();
              
              // 刷新提交列表
              if (this.$refs.submissionList) {
                this.$refs.submissionList.fetchSubmissions();
              }
            } else {
              this.$message.error(response.message || '重新提交失败');
            }
          })
          .catch(error => {
            console.error('重新提交失败:', error);
            this.$message.error('重新提交失败，请稍后再试');
          });
      }).catch(() => {
        // 用户取消操作
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
  }
};
</script>

<style lang="scss" scoped>
.problem-detail-container {
  padding: 0;
  height: 100%;
  
  .problem-detail-card {
    height: 100%;
    border-radius: 4px;
    box-shadow: 0 2px 12px 0 rgba(0, 0, 0, 0.1);
    
    .card-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      font-size: 18px;
      font-weight: bold;
      
      .card-header-info {
        display: flex;
        align-items: center;
        gap: 10px;
      }
    }
    
    .content-section {
      margin-top: 20px;
      
      h3 {
        margin-top: 20px;
        margin-bottom: 10px;
        font-size: 18px;
        font-weight: 600;
        color: #333;
      }
      
      .markdown-content {
        line-height: 1.6;
        white-space: pre-wrap;
        margin-bottom: 15px;
      }
    }
    
    .example-section {
      display: flex;
      flex-direction: column;
      margin-bottom: 20px;
      
      .example-input,
      .example-output {
        margin-bottom: 15px;
        
        .example-title {
          font-weight: 600;
          margin-bottom: 5px;
        }
        
        pre {
          background-color: #f5f7fa;
          padding: 10px;
          border-radius: 4px;
          overflow-x: auto;
          line-height: 1.5;
          max-height: 200px;
        }
      }
    }
    
    .submit-section,
    .submissions-section,
    .discussions-section {
      padding: 15px 0;
      
      h3 {
        margin-top: 0;
        margin-bottom: 15px;
      }
      
      .language-selector {
        margin-bottom: 15px;
        display: flex;
        align-items: center;
        
        span {
          margin-right: 10px;
        }
      }
      
      .code-editor-wrapper {
        margin-bottom: 15px;
        
        .code-editor {
          font-family: 'Courier New', monospace;
        }
      }
      
      .submit-button-container {
        text-align: right;
      }
    }
    
    .submissions-section {
      padding: 15px 0;
      margin-top: 20px;
      
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
    }
    
    ::v-deep .el-tabs__header {
      margin-bottom: 20px;
    }
  }
}

@media screen and (max-width: 768px) {
  .problem-detail-container {
    padding: 10px;
    
    .problem-detail-card {
      .card-header {
        flex-direction: column;
        align-items: flex-start;
        
        .card-header-info {
          width: 100%;
          margin-top: 10px;
          justify-content: space-between;
        }
      }
      
      .example-section {
        overflow-x: auto;
      }
    }
  }
}
</style> 