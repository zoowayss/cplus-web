<template>
  <div class="leetcode-layout" :class="{ 'menu-collapsed': isMenuCollapsed }">
    <!-- 自定义头部导航 -->
    <div class="problem-header">
      <div class="header-left">
        <el-button icon="el-icon-back" size="small" type="text" @click="goBackToList">返回题目列表</el-button>
        <span class="problem-title">{{ problem.title }}</span>
        <el-tag v-if="problem.difficulty" :type="getDifficultyType(problem.difficulty)" size="small">{{ problem.difficulty }}</el-tag>
      </div>
      <div class="header-right">
        <el-dropdown trigger="click">
          <span class="el-dropdown-link">
            更多 <i class="el-icon-arrow-down el-icon--right"></i>
          </span>
          <el-dropdown-menu slot="dropdown">
            <el-dropdown-item>收藏题目</el-dropdown-item>
            <el-dropdown-item>分享题目</el-dropdown-item>
          </el-dropdown-menu>
        </el-dropdown>
      </div>
    </div>

    <div class="problem-content" v-loading="loading">
      <!-- 使用LeetCode类似的双栏布局 -->
      <div class="content-split">
        <!-- 左侧：题目描述区域 -->
        <div class="left-panel">
          <el-tabs v-model="leftTabActive" type="border-card">
            <el-tab-pane label="题目描述" name="description">
              <ProblemDescription v-if="problem.id" :problem="problem" />
              <el-empty v-else-if="!loading" description="题目不存在或已被删除" />
            </el-tab-pane>
            <el-tab-pane label="讨论" name="discussions">
              <div class="discussions-section">
                <DiscussionList :problemId="Number(problemId)" />
              </div>
            </el-tab-pane>
          </el-tabs>
        </div>

        <!-- 右侧：代码编辑区域 -->
        <div class="right-panel">
          <el-tabs v-model="rightTabActive" type="border-card">
            <el-tab-pane label="代码编辑器" name="editor">
              <CodeSubmission 
                :code.sync="code"
                :submitting="submitting"
                :problem="problem"
                @submit="submitCode"
              />
            </el-tab-pane>
            <el-tab-pane label="提交记录" name="submissions">
              <div class="submissions-section">
                <h3>我的提交记录</h3>
                
                <SubmissionDetail
                  v-if="currentSubmissionId"
                  :submissionId="currentSubmissionId"
                  :submissionDetail="currentSubmissionDetail"
                  @close="closeSubmissionDetail"
                  @resubmit="resubmitCode"
                />
                
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
          </el-tabs>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { getProblemDetail, submitCode, getUserProblemSubmissions } from '@/api/problem';
import SubmissionList from '@/views/problems/SubmissionList.vue';
import DiscussionList from '@/views/problems/DiscussionList.vue';
import request from '@/utils/request';

// 导入拆分后的组件
import { ProblemDescription, CodeSubmission, SubmissionDetail } from './components';

export default {
  name: 'ProblemDetail',
  components: {
    SubmissionList,
    DiscussionList,
    ProblemDescription,
    CodeSubmission,
    SubmissionDetail
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
      isMenuCollapsed: true, // 默认折叠菜单
      loading: false,
      submitting: false,
      loadingSubmissions: false,
      problem: {},
      problemId: null,
      leftTabActive: 'description',
      rightTabActive: 'editor',
      code: '',
      submissions: [],
      pagination: {
        page: 1,
        limit: 10,
        total: 0
      },
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

    // 通知父组件折叠菜单
    this.notifyCollapseMenu();
  },
  mounted() {
    console.log('ProblemDetail mounted, params:', this.params);
    
    // 监听参数变化
    this.$watch(
      'params.problemId',
      (newId) => {
        console.log('参数problemId变化:', newId);
        if (newId && newId !== this.problemId) {
          this.problemId = parseInt(newId);
          this.fetchProblemDetail();
          this.fetchUserSubmissions();
        }
      }
    );
    
    // 处理导航守卫，监听路由变化
    this.$watch('$route.params.id', (newId) => {
      console.log('路由参数id变化:', newId);
      if (newId && newId !== this.problemId) {
        this.problemId = parseInt(newId);
        this.fetchProblemDetail();
        this.fetchUserSubmissions();
      }
    });
  },
  methods: {
    // 通知父组件折叠菜单
    notifyCollapseMenu() {
      // 如果在UserHome或Home中
      if (this.$parent && (this.$parent.$options.name === 'UserHome' || this.$parent.$options.name === 'Home')) {
        // 设置父组件的isCollapse为true
        if (!this.$parent.isCollapse) {
          this.$parent.isCollapse = true;
        }
      }
    },
    // 获取题目详情
    fetchProblemDetail() {
      this.loading = true;
      getProblemDetail(parseInt(this.problemId))
        .then(response => {
          if (response.status === 'ok' && response.problem) {
            this.problem = response.problem;
          } else {
            this.$message.error(response.message || '获取题目详情失败');
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
        // problemId 转为int 类型
        problem_id: parseInt(this.problemId),
        code: this.code,
        language: 'cpp'
      };
      
      submitCode(data)
        .then(response => {
          // 判断响应状态
          if (response.status === 'ok' || (response.message && response.message.includes('正在评测'))) {
            this.$message.success('代码提交成功，正在评测');
            
            // 切换到提交记录选项卡
            this.rightTabActive = 'submissions';
            
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
            this.rightTabActive = 'submissions';
            
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
    }
  }
};
</script>

<style lang="scss" scoped>
.leetcode-layout {
  height: 100%;
  display: flex;
  flex-direction: column;
  background-color: #f9fafb;
  
  // 当菜单折叠时的样式调整
  &.menu-collapsed {
    flex: 1;
    width: 100%;
  }
  
  .problem-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 10px 20px;
    background-color: #fff;
    box-shadow: 0 1px 3px rgba(0, 0, 0, 0.1);
    z-index: 10;
    
    .header-left {
      display: flex;
      align-items: center;
      gap: 12px;
      
      .problem-title {
        font-size: 18px;
        font-weight: bold;
      }
    }
    
    .header-right {
      display: flex;
      align-items: center;
      
      .el-dropdown-link {
        cursor: pointer;
        color: #409EFF;
      }
    }
  }
  
  .problem-content {
    flex: 1;
    overflow: hidden;
    padding: 16px;
    
    .content-split {
      display: flex;
      height: 100%;
      gap: 16px;
      
      .left-panel, .right-panel {
        flex: 1;
        height: 100%;
        overflow: hidden;
        display: flex;
        flex-direction: column;
      }
      
      .left-panel {
        min-width: 40%;
        max-width: 50%;
      }
      
      .right-panel {
        min-width: 50%;
      }
      
      // 为标签页容器设置overflow样式，使内容可滚动
      ::v-deep .el-tabs {
        display: flex;
        flex-direction: column;
        height: 100%;
        
        .el-tabs__header {
          margin: 0;
        }
        
        .el-tabs__content {
          flex: 1;
          overflow-y: auto;
          padding: 15px;
        }
      }
    }
  }
  
  // 提交记录部分样式
  .submissions-section {
    h3 {
      margin-top: 0;
      margin-bottom: 15px;
    }
  }
}

// 响应式布局
@media screen and (max-width: 1024px) {
  .leetcode-layout {
    .problem-content {
      .content-split {
        flex-direction: column;
        
        .left-panel, .right-panel {
          max-width: 100%;
          min-width: 100%;
          height: auto;
        }
        
        .left-panel {
          height: 400px; // 固定高度，避免挤压代码编辑区域
        }
      }
    }
  }
}
</style> 