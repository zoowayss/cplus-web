<template>
  <div class="problem-detail-container">
    <el-card class="problem-detail-card" v-loading="loading" shadow="never">
      <div slot="header" class="card-header" v-if="!isDialogMode">
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
              <div class="difficulty-tag" v-if="isDialogMode">
                <el-tag :type="getDifficultyType(problem.difficulty)" size="small">{{ problem.difficulty }}</el-tag>
              </div>
              
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
              
              <!-- 提交记录表格 -->
              <el-table :data="submissions" border style="width: 100%" v-loading="loadingSubmissions">
                <el-table-column prop="id" label="ID" width="80" />
                <el-table-column prop="language" label="语言" width="100" />
                <el-table-column prop="status" label="状态" width="120">
                  <template slot-scope="scope">
                    <el-tag :type="getSubmissionStatusType(scope.row.status)" size="small">
                      {{ scope.row.status }}
                    </el-tag>
                  </template>
                </el-table-column>
                <el-table-column prop="runtime" label="运行时间" width="120">
                  <template slot-scope="scope">
                    {{ scope.row.runtime }} ms
                  </template>
                </el-table-column>
                <el-table-column prop="memory" label="内存占用" width="120">
                  <template slot-scope="scope">
                    {{ scope.row.memory }} KB
                  </template>
                </el-table-column>
                <el-table-column prop="created_at" label="提交时间" width="180">
                  <template slot-scope="scope">
                    {{ formatDate(scope.row.created_at) }}
                  </template>
                </el-table-column>
              </el-table>
              
              <!-- 分页组件 -->
              <div class="pagination-container" v-if="submissions.length > 0">
                <el-pagination
                  @size-change="handleSizeChange"
                  @current-change="handleCurrentChange"
                  :current-page="pagination.page"
                  :page-sizes="[5, 10, 20, 50]"
                  :page-size="pagination.limit"
                  layout="total, sizes, prev, pager, next, jumper"
                  :total="pagination.total"
                >
                </el-pagination>
              </div>
              
              <!-- 空状态 -->
              <el-empty v-if="submissions.length === 0 && !loadingSubmissions" description="暂无提交记录" />
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

export default {
  name: 'ProblemDetail',
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
      }
    };
  },
  created() {
    // 优先使用从 props 传递的 problemId
    if (this.params && this.params.problemId) {
      this.problemId = parseInt(this.params.problemId);
      this.fetchProblemDetail();
      this.fetchUserSubmissions();
      return;
    }
    
    // 从URL查询参数中获取题目ID
    const queryProblemId = this.$route.query.problemId;
    if (queryProblemId) {
      this.problemId = parseInt(queryProblemId);
      this.fetchProblemDetail();
      this.fetchUserSubmissions();
      return;
    }
    
    // 回退到从路由中获取题目ID
    const { id } = this.$route.params;
    if (id) {
      this.problemId = parseInt(id);
      this.fetchProblemDetail();
      this.fetchUserSubmissions();
    } else {
      this.$message.error('无效的题目ID');
      this.$router.push('/problems');
    }
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
          // 只要API返回了响应（无论状态如何），我们都认为代码已经成功提交到服务器
          console.log('提交代码响应:', response);
          
          // 判断响应状态
          if (response.status === 'ok' || (response.message && response.message.includes('正在评测'))) {
            this.$message.success('代码提交成功，正在评测');
            
            // 添加临时提交记录到列表顶部
            const tempSubmission = {
              id: response.submission_id || Date.now(),
              language: this.getLanguageDisplay(this.language),
              status: '评测中',
              runtime: 0,
              memory: 0,
              created_at: Math.floor(Date.now() / 1000)
            };
            
            // 添加到提交记录列表顶部
            this.submissions.unshift(tempSubmission);
            
            // 切换到提交记录选项卡
            this.activeTab = 'submissions';
            
            // 延迟刷新提交记录，获取最新评测结果
            setTimeout(() => {
              this.fetchUserSubmissions();
            }, 2000);
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
            
            // 延迟刷新提交记录，获取最新评测结果
            setTimeout(() => {
              this.fetchUserSubmissions();
            }, 2000);
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
        case '评测中':
          return 'info';
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
      // 触发关闭事件，用于在弹窗模式下关闭弹窗
      this.$emit('close');
      
      // 如果是在 UserHome 中作为组件加载的，通过父组件方法返回
      if (this.$parent && typeof this.$parent.handleSelect === 'function') {
        this.$parent.handleSelect('problems');
      } else if (this.$parent && typeof this.$parent.$parent !== 'undefined' && 
                typeof this.$parent.$parent.handleSelect === 'function') {
        // 可能嵌套在多层组件中
        this.$parent.$parent.handleSelect('problems');
      } else {
        // 否则使用路由
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
    }
  }
};
</script>

<style lang="scss" scoped>
.problem-detail-container {
  padding: 20px;
  
  .problem-detail-card {
    .card-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      
      .card-header-info {
        display: flex;
        align-items: center;
        gap: 10px;
      }
    }
    
    .content-section {
      padding: 10px 0;
      
      .difficulty-tag {
        margin-bottom: 15px;
      }
      
      h3 {
        margin-top: 20px;
        margin-bottom: 10px;
        font-weight: bold;
        color: #303133;
        border-bottom: 1px solid #ebeef5;
        padding-bottom: 10px;
      }
      
      .markdown-content {
        line-height: 1.6;
        color: #606266;
        white-space: pre-wrap;
      }
      
      .example-section {
        display: flex;
        flex-direction: column;
        gap: 15px;
        margin-bottom: 20px;
        
        .example-input, .example-output {
          .example-title {
            font-weight: bold;
            margin-bottom: 5px;
          }
          
          pre {
            background-color: #f5f7fa;
            border: 1px solid #ebeef5;
            border-radius: 4px;
            padding: 15px;
            overflow-x: auto;
            margin: 0;
          }
        }
      }
    }
    
    .submit-section {
      padding: 10px 0;
      
      h3 {
        margin-top: 20px;
        margin-bottom: 15px;
        font-weight: bold;
        color: #303133;
      }
      
      .language-selector {
        margin-bottom: 15px;
        display: flex;
        align-items: center;
        gap: 10px;
      }
      
      .code-editor-wrapper {
        margin-bottom: 20px;
        
        .code-editor {
          font-family: "Courier New", Courier, monospace;
          font-size: 14px;
        }
      }
      
      .submit-button-container {
        text-align: right;
      }
    }
    
    .submissions-section {
      padding: 10px 0;
      
      h3 {
        margin-top: 20px;
        margin-bottom: 15px;
        font-weight: bold;
        color: #303133;
      }
      
      .pagination-container {
        margin-top: 20px;
        text-align: right;
      }
    }
    
    ::v-deep .el-tabs__header {
      margin-bottom: 20px;
    }
  }
}

// 对话框模式下的特殊样式
::v-deep .el-dialog__body {
  padding: 0;
  
  .problem-detail-container {
    padding: 0;
  }
  
  .problem-detail-card {
    box-shadow: none;
    border: none;
    
    .el-card__body {
      padding: 20px;
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
        gap: 10px;
        
        .card-header-info {
          width: 100%;
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