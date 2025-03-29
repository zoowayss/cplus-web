<template>
  <div class="problem-list-container">
    <el-card class="problem-list-card">
      <div slot="header" class="card-header">
        <span>题目列表</span>
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
        <el-table-column label="解题率" width="100">
          <template slot-scope="scope">
            {{ scope.row.acceptance_rate || '0%' }}
          </template>
        </el-table-column>
        <el-table-column label="状态" width="120">
          <template slot-scope="scope">
            <el-tag
              v-if="scope.row.user_status"
              :type="getProblemStatusType(scope.row.user_status)"
              size="small">
              {{ getProblemStatusText(scope.row.user_status) }}
            </el-tag>
            <span v-else>未尝试</span>
          </template>
        </el-table-column>
        <el-table-column label="操作" width="100">
          <template slot-scope="scope">
            <el-button type="text" size="small" @click="viewProblem(scope.row)">
              查看
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
  </div>
</template>

<script>
import { getProblems } from '@/api/problem';

export default {
  name: 'ProblemList',
  data() {
    return {
      loading: false,
      problems: [],
      total: 0,
      currentPage: 1,
      pageSize: 10,
      searchQuery: '',
      difficultyFilter: '',
      // 模拟的解题状态数据，实际应从后端获取
      mockUserStatus: {
        1: 'accepted',
        2: 'attempted',
        3: 'accepted'
      },
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
        status: 1 // 只显示启用的题目
      };
      
      getProblems(params)
        .then(response => {
          if (response.status === 'ok') {
            // 添加解题率和用户解题状态（实际项目中这些数据应该从后端获取）
            this.problems = (response.problems || []).map(problem => {
              return {
                ...problem,
                acceptance_rate: this.getRandomAcceptanceRate(),
                user_status: this.mockUserStatus[problem.id] || null
              };
            });
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
    
    // 生成随机解题率（模拟数据）
    getRandomAcceptanceRate() {
      const rate = Math.floor(Math.random() * 100);
      return rate + '%';
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
    
    // 查看题目 - 打开弹窗
    viewProblem(problem) {
      console.log('查看题目，ID:', problem.id);
      
      // 检查是否在用户主页或管理员主页环境下
      if (this.$parent && typeof this.$parent.viewProblem === 'function') {
        // 如果在主页环境中且父组件提供了viewProblem方法，使用父组件方法
        console.log('使用父组件方法查看题目');
        this.$parent.viewProblem(problem.id);
      } else {
        // 否则直接导航到题目详情页
        console.log('导航到题目详情页');
        this.$router.push(`/problems/${problem.id}`);
      }
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
    
    // 根据用户解题状态返回不同的标签类型
    getProblemStatusType(status) {
      switch (status) {
        case 'accepted':
          return 'success';
        case 'attempted':
          return 'warning';
        default:
          return 'info';
      }
    },
    
    // 根据用户解题状态返回文本
    getProblemStatusText(status) {
      switch (status) {
        case 'accepted':
          return '已通过';
        case 'attempted':
          return '尝试过';
        default:
          return '未知';
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
}
</style> 