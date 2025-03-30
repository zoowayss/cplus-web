<template>
  <div class="problem-list-container">
    <el-card class="problem-list-card">
      <div slot="header" class="card-header">
        <span>题目管理</span>
        <el-button type="primary" size="small" @click="$router.push('/admin/problems/create')">
          <i class="el-icon-plus"></i> 添加题目
        </el-button>
      </div>
      
      <!-- 搜索和筛选 -->
      <div class="search-bar">
        <el-input
          v-model="searchQuery"
          placeholder="搜索题目标题或描述"
          prefix-icon="el-icon-search"
          clearable
          @keyup.enter.native="fetchProblems"
          @clear="handleClear"
          class="search-input"
        />
        <el-select v-model="difficultyFilter" placeholder="难度筛选" clearable @change="fetchProblems" class="filter-select">
          <el-option label="简单" value="简单" />
          <el-option label="中等" value="中等" />
          <el-option label="困难" value="困难" />
        </el-select>
        <el-select v-model="statusFilter" placeholder="状态筛选" clearable @change="fetchProblems" class="filter-select">
          <el-option label="启用" :value="1" />
          <el-option label="禁用" :value="0" />
        </el-select>
        <el-button type="primary" @click="fetchProblems">
          <i class="el-icon-search"></i> 查询
        </el-button>
      </div>
      
      <!-- 题目列表表格 -->
      <el-table :data="problems" border style="width: 100%" v-loading="loading">
        <el-table-column prop="id" label="ID" width="80" />
        <el-table-column prop="title" label="标题" min-width="200" />
        <el-table-column prop="difficulty" label="难度" width="100">
          <template slot-scope="scope">
            <el-tag 
              :type="getDifficultyType(scope.row.difficulty)" 
              size="small">
              {{ scope.row.difficulty }}
            </el-tag>
          </template>
        </el-table-column>
        <el-table-column label="状态" width="100">
          <template slot-scope="scope">
            <el-tag 
              :type="scope.row.status === 1 ? 'success' : 'info'" 
              size="small">
              {{ scope.row.status === 1 ? '启用' : '禁用' }}
            </el-tag>
          </template>
        </el-table-column>
        <el-table-column label="创建时间" width="180">
          <template slot-scope="scope">
            {{ formatDate(scope.row.created_at) }}
          </template>
        </el-table-column>
        <el-table-column label="操作" width="250">
          <template slot-scope="scope">
            <el-button type="text" size="small" @click="editProblem(scope.row)">
              编辑
            </el-button>
            <el-button type="text" size="small" @click="manageTestCases(scope.row)">
              测试用例
            </el-button>
            <el-button 
              type="text" 
              size="small" 
              :class="{ 'danger-text': scope.row.status === 1, 'success-text': scope.row.status === 0 }"
              @click="toggleStatus(scope.row)">
              {{ scope.row.status === 1 ? '禁用' : '启用' }}
            </el-button>
            <el-button type="text" size="small" class="danger-text" @click="confirmDelete(scope.row)">
              删除
            </el-button>
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
    </el-card>
    
    <!-- 删除确认弹窗 -->
    <el-dialog
      title="确认删除"
      :visible.sync="deleteDialogVisible"
      width="30%">
      <span>确定要删除题目 "{{ problemToDelete ? problemToDelete.title : '' }}" 吗？此操作不可恢复。</span>
      <span slot="footer" class="dialog-footer">
        <el-button @click="deleteDialogVisible = false">取消</el-button>
        <el-button type="danger" @click="deleteProblem">确定删除</el-button>
      </span>
    </el-dialog>
  </div>
</template>

<script>
import { getProblems, deleteProblem, updateProblem } from '@/api/problem';

export default {
  name: 'AdminProblemList',
  data() {
    return {
      loading: false,
      problems: [],
      total: 0,
      currentPage: 1,
      pageSize: 10,
      searchQuery: '',
      difficultyFilter: '',
      statusFilter: '',
      deleteDialogVisible: false,
      problemToDelete: null
    };
  },
  created() {
    this.fetchProblems();
  },
  methods: {
    // 获取题目列表
    fetchProblems() {
      this.loading = true;
      
      const params = {
        offset: (this.currentPage - 1) * this.pageSize,
        limit: this.pageSize,
        search: this.searchQuery,
        difficulty: this.difficultyFilter,
        status: this.statusFilter
      };
      
      getProblems(params)
        .then(response => {
          if (response.status === 'ok') {
            this.problems = response.problems || [];
            this.total = response.total || 0;
          } else {
            this.$message.error(response.message || '获取题目列表失败');
          }
        })
        .catch(error => {
          console.error('获取题目列表失败:', error);
          this.$message.error('获取题目列表失败，请稍后再试');
        })
        .finally(() => {
          this.loading = false;
        });
    },
    
    // 每页数量变化
    handleSizeChange(val) {
      this.pageSize = val;
      this.fetchProblems();
    },
    
    // 页码变化
    handleCurrentChange(val) {
      this.currentPage = val;
      this.fetchProblems();
    },
    
    // 清除搜索条件
    handleClear() {
      this.fetchProblems();
    },
    
    // 查看题目
    viewProblem(problem) {
      // 检查是否在父组件的环境中（即Home组件内）
      if (this.$parent && typeof this.$parent.handleComponentNavigation === 'function') {
        // 使用父组件的处理方法加载题目详情
        this.$parent.handleComponentNavigation({ 
          component: 'problem-detail', 
          problemId: problem.id 
        });
      } else if (this.$parent && typeof this.$parent.editProblem === 'function') {
        // 调用父组件的编辑方法（在管理员界面中应该是这个）
        this.$parent.editProblem(problem.id);
      } else {
        // 后备方案：直接导航到管理员题目编辑页面
        this.$router.push({
          path: `/admin/problems/edit/${problem.id}`,
          query: { fromAdmin: true }
        }).catch(err => {
          // 忽略导航重复错误
          if (err.name !== 'NavigationDuplicated') {
            throw err;
          }
        });
      }
    },
    
    // 编辑题目
    editProblem(problem) {
      // 检查是否在父组件的环境中
      if (this.$parent && typeof this.$parent.editProblem === 'function') {
        // 使用父组件的编辑方法
        this.$parent.editProblem(problem.id);
      } else {
        // 后备方案，添加 fromAdmin 查询参数
        this.$router.push({
          path: `/admin/problems/edit/${problem.id}`,
          query: { fromAdmin: true }
        }).catch(err => {
          // 忽略导航重复错误
          if (err.name !== 'NavigationDuplicated') {
            throw err;
          }
        });
      }
    },
    
    // 管理测试用例
    manageTestCases(problem) {
      // 检查是否在父组件的环境中
      if (this.$parent && typeof this.$parent.manageProblemTestCases === 'function') {
        // 使用父组件的管理测试用例方法
        this.$parent.manageProblemTestCases(problem.id);
      } else {
        // 后备方案，添加 fromAdmin 查询参数
        this.$router.push({
          path: `/admin/problems/testcases/${problem.id}`,
          query: { fromAdmin: true }
        }).catch(err => {
          // 忽略导航重复错误
          if (err.name !== 'NavigationDuplicated') {
            throw err;
          }
        });
      }
    },
    
    // 改变题目状态（启用/禁用）
    toggleStatus(problem) {
      const newStatus = problem.status === 1 ? 0 : 1;
      const statusText = newStatus === 1 ? '启用' : '禁用';
      
      // 创建更新对象
      const updateData = {
        id: problem.id,
        status: newStatus
      };
      
      updateProblem(problem.id, updateData)
        .then(response => {
          if (response.status === 'ok') {
            this.$message.success(`${statusText}题目成功`);
            // 更新本地数据
            problem.status = newStatus;
          } else {
            this.$message.error(response.message || `${statusText}题目失败`);
          }
        })
        .catch(error => {
          console.error(`${statusText}题目失败:`, error);
          this.$message.error(`${statusText}题目失败，请稍后再试`);
        });
    },
    
    // 确认删除对话框
    confirmDelete(problem) {
      this.problemToDelete = problem;
      this.deleteDialogVisible = true;
    },
    
    // 删除题目
    deleteProblem() {
      if (!this.problemToDelete) return;
      
      console.log('尝试删除题目:', this.problemToDelete.id);
      
      deleteProblem(this.problemToDelete.id)
        .then(response => {
          console.log('删除题目响应:', response);
          if (response.status === 'ok') {
            this.$message.success('删除题目成功');
            // 关闭对话框
            this.deleteDialogVisible = false;
            // 重新加载题目列表
            this.fetchProblems();
          } else {
            this.$message.error(response.message || '删除题目失败');
          }
        })
        .catch(error => {
          console.error('删除题目失败:', error);
          // 显示详细错误信息
          const errorMessage = error.response?.data?.message || error.message || '未知错误';
          this.$message({
            type: 'error',
            message: `删除题目失败: ${errorMessage}`,
            duration: 5000,
            showClose: true
          });
          
          // 如果是权限问题，提示用户
          if (error.response?.status === 403) {
            this.$message({
              type: 'warning',
              message: '您可能没有权限删除该题目，请联系管理员',
              duration: 5000
            });
          }
        });
    },
    
    // 格式化日期
    formatDate(timestamp) {
      if (!timestamp) return '';
      
      const date = new Date(timestamp * 1000);
      return date.toLocaleString();
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
    }
  }
};
</script>

<style lang="scss" scoped>
.problem-list-container {
  padding: 20px;
  
  .problem-list-card {
    .card-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
    
    .search-bar {
      margin-bottom: 20px;
      display: flex;
      align-items: center;
      flex-wrap: wrap;
      gap: 10px;
      
      .search-input {
        width: 300px;
      }
      
      .filter-select {
        width: 150px;
      }
    }
    
    .pagination-container {
      margin-top: 20px;
      text-align: right;
    }
  }
  
  .danger-text {
    color: #f56c6c !important;
  }
  
  .success-text {
    color: #67c23a !important;
  }
}
</style> 