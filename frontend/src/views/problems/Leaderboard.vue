<template>
  <div class="leaderboard-container">
    <el-card class="leaderboard-card">
      <div slot="header" class="card-header">
        <span>排行榜</span>
        <div class="refresh-button">
          <el-button size="small" icon="el-icon-refresh" @click="fetchLeaderboard">刷新</el-button>
        </div>
      </div>
      
      <!-- 选择范围 -->
      <div class="filter-bar">
        <el-radio-group v-model="timeRange" @change="handleTimeRangeChange">
          <el-radio-button label="all">全部</el-radio-button>
          <el-radio-button label="month">本月</el-radio-button>
          <el-radio-button label="week">本周</el-radio-button>
          <el-radio-button label="day">今日</el-radio-button>
        </el-radio-group>
      </div>
      
      <!-- 排行榜表格 -->
      <el-table 
        :data="leaderboardData" 
        border 
        style="width: 100%" 
        v-loading="loading"
        :row-class-name="tableRowClassName"
      >
        <el-table-column label="排名" width="80">
          <template slot-scope="scope">
            <div class="rank-cell">
              <template v-if="scope.$index < 3">
                <img :src="getRankIcon(scope.$index + 1)" class="rank-icon" />
              </template>
              <template v-else>
                {{ scope.$index + 1 }}
              </template>
            </div>
          </template>
        </el-table-column>
        <el-table-column prop="username" label="用户" min-width="150">
          <template slot-scope="scope">
            <div class="user-cell">
              <el-avatar :size="small" class="user-avatar">{{ scope.row.username.charAt(0) }}</el-avatar>
              <span class="username">{{ scope.row.username }}</span>
              <span v-if="scope.row.is_current_user" class="current-user-tag">(我)</span>
            </div>
          </template>
        </el-table-column>
        <el-table-column prop="solved_count" label="已解决题目" width="120" sortable />
        <el-table-column prop="submission_count" label="提交次数" width="120" sortable />
        <el-table-column prop="acceptance_rate" label="通过率" width="120">
          <template slot-scope="scope">
            {{ scope.row.acceptance_rate }}%
          </template>
        </el-table-column>
        <el-table-column prop="score" label="积分" width="100" sortable />
        <el-table-column label="难度分布" min-width="200">
          <template slot-scope="scope">
            <div class="difficulty-distribution">
              <el-tooltip content="简单题" placement="top">
                <el-tag type="success" size="small">{{ scope.row.easy_count }}</el-tag>
              </el-tooltip>
              <el-tooltip content="中等题" placement="top">
                <el-tag type="warning" size="small">{{ scope.row.medium_count }}</el-tag>
              </el-tooltip>
              <el-tooltip content="困难题" placement="top">
                <el-tag type="danger" size="small">{{ scope.row.hard_count }}</el-tag>
              </el-tooltip>
            </div>
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
      
      <!-- 关于排名的说明 -->
      <div class="ranking-info">
        <h3>排名规则</h3>
        <p>1. 排名首先按照已解决的题目数量从高到低排序；</p>
        <p>2. 在解决题目数量相同的情况下，按照通过率从高到低排序；</p>
        <p>3. 在解决题目数量和通过率都相同的情况下，按照提交次数从少到多排序。</p>
      </div>
    </el-card>
  </div>
</template>

<script>
import { getLeaderboard } from '@/api/leaderboard'

export default {
  name: 'Leaderboard',
  data() {
    return {
      loading: false,
      leaderboardData: [],
      currentPage: 1,
      pageSize: 10,
      total: 0,
      timeRange: 'all'
    };
  },
  created() {
    this.fetchLeaderboard();
  },
  methods: {
    // 获取排行榜数据
    fetchLeaderboard() {
      this.loading = true;
      
      const params = {
        offset: (this.currentPage - 1) * this.pageSize,
        limit: this.pageSize,
        time_range: this.timeRange
      };
      
      getLeaderboard(params)
        .then(response => {
          console.log('获取排行榜数据:', response);
          if (response.status === 'ok') {
            // 检查数据结构
            if (response.data && response.data.leaderboard) {
              this.leaderboardData = response.data.leaderboard || [];
              this.total = response.data.total || 0;
            } else if (response.leaderboard) {
              // 兼容不同的数据结构
              this.leaderboardData = response.leaderboard || [];
              this.total = response.total || 0;
            } else {
              // 无数据时展示空数组
              this.leaderboardData = [];
              this.total = 0;
              console.warn('排行榜数据格式不正确', response);
            }
          } else {
            this.$message.error(response.message || '获取排行榜失败');
          }
        })
        .catch(error => {
          console.error('获取排行榜失败:', error);
          this.$message.error('获取排行榜失败，请稍后再试');
        })
        .finally(() => {
          this.loading = false;
        });
    },
    
    // 处理时间范围变化
    handleTimeRangeChange() {
      this.currentPage = 1;
      this.fetchLeaderboard();
    },
    
    // 每页数量变化
    handleSizeChange(val) {
      this.pageSize = val;
      this.fetchLeaderboard();
    },
    
    // 页码变化
    handleCurrentChange(val) {
      this.currentPage = val;
      this.fetchLeaderboard();
    },
    
    // 表格行样式
    tableRowClassName({ row, rowIndex }) {
      if (row.is_current_user) {
        return 'current-user-row';
      }
      return '';
    },
    
    // 获取排名图标
    getRankIcon(rank) {
      const icons = {
        1: 'https://assets.leetcode.com/static_assets/public/images/medals/gold.png',
        2: 'https://assets.leetcode.com/static_assets/public/images/medals/silver.png',
        3: 'https://assets.leetcode.com/static_assets/public/images/medals/bronze.png'
      };
      return icons[rank] || '';
    }
  }
};
</script>

<style lang="scss" scoped>
.leaderboard-container {
  padding: 20px;
  
  .leaderboard-card {
    .card-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
    
    .filter-bar {
      margin-bottom: 20px;
      text-align: center;
    }
    
    .rank-cell {
      display: flex;
      justify-content: center;
      align-items: center;
      font-weight: bold;
      
      .rank-icon {
        width: 20px;
        height: 20px;
      }
    }
    
    .user-cell {
      display: flex;
      align-items: center;
      
      .user-avatar {
        margin-right: 10px;
      }
      
      .username {
        font-weight: bold;
      }
      
      .current-user-tag {
        margin-left: 5px;
        color: #409EFF;
      }
    }
    
    .difficulty-distribution {
      display: flex;
      gap: 8px;
    }
    
    .pagination-container {
      margin-top: 20px;
      text-align: right;
    }
    
    .ranking-info {
      margin-top: 30px;
      padding: 15px;
      background-color: #f5f7fa;
      border-radius: 4px;
      
      h3 {
        margin-top: 0;
        margin-bottom: 10px;
        font-size: 16px;
        color: #303133;
      }
      
      p {
        margin: 5px 0;
        color: #606266;
        font-size: 14px;
      }
    }
  }
}

// 当前用户行高亮
::v-deep .current-user-row {
  background-color: #ecf5ff;
}

@media screen and (max-width: 768px) {
  .leaderboard-container {
    padding: 10px;
    
    .filter-bar {
      overflow-x: auto;
      white-space: nowrap;
      padding-bottom: 10px;
    }
    
    .difficulty-distribution {
      flex-wrap: wrap;
    }
  }
}
</style> 