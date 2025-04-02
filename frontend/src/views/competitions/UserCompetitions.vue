<template>
  <div class="user-competitions-container">
    <el-card class="competitions-card">
      <div slot="header" class="card-header">
        <span>当前比赛</span>
      </div>
      
      <!-- 比赛列表 -->
      <template v-if="competitions.length > 0">
        <div class="competition-list">
          <el-card v-for="competition in competitions" :key="competition.id" class="competition-item" shadow="hover">
            <div class="competition-header">
              <h3 class="competition-title">{{ competition.title }}</h3>
              <el-tag type="success" size="small">进行中</el-tag>
            </div>
            <p class="competition-description">{{ competition.description }}</p>
            <div class="competition-info">
              <div class="info-item">
                <i class="el-icon-time"></i>
                <span>开始时间: {{ formatDate(competition.startDate) }}</span>
              </div>
              <div class="info-item">
                <i class="el-icon-timer"></i>
                <span>结束时间: {{ formatDate(competition.endDate) }}</span>
              </div>
              <div class="info-item">
                <i class="el-icon-document"></i>
                <span>题目数量: {{ competition.problems.length }}</span>
              </div>
              <div class="competition-countdown">
                <span>倒计时: </span>
                <span class="countdown-time">{{ getCountdown(competition.endDate) }}</span>
              </div>
            </div>
            <div class="competition-actions">
              <el-button type="primary" size="small" @click="viewCompetitionProblems(competition)">
                查看题目
              </el-button>
            </div>
          </el-card>
        </div>
      </template>
      
      <!-- 空数据提示 -->
      <el-empty v-else description="暂无进行中的比赛"></el-empty>
    </el-card>
    
    <!-- 比赛题目对话框 -->
    <el-dialog 
      :title="currentCompetition ? currentCompetition.title + ' - 比赛题目' : '比赛题目'" 
      :visible.sync="problemsDialogVisible" 
      width="65%">
      <div v-if="currentCompetition" class="competition-problems">
        <div class="dialog-info">
          <p><i class="el-icon-timer"></i> 倒计时: <span class="countdown-time">{{ getCountdown(currentCompetition.endDate) }}</span></p>
          <p>{{ currentCompetition.description }}</p>
        </div>
        
        <el-table :data="competitionProblems" border style="width: 100%">
          <el-table-column type="index" label="#" width="50"></el-table-column>
          <el-table-column prop="id" label="ID" width="80"></el-table-column>
          <el-table-column prop="title" label="题目" min-width="200"></el-table-column>
          <el-table-column prop="difficulty" label="难度" width="100">
            <template slot-scope="scope">
              <el-tag 
                :type="getDifficultyType(scope.row.difficulty)" 
                size="small">
                {{ scope.row.difficulty }}
              </el-tag>
            </template>
          </el-table-column>
          <el-table-column label="操作" width="120">
            <template slot-scope="scope">
              <el-button type="text" size="small" @click="viewProblem(scope.row)">
                查看
              </el-button>
            </template>
          </el-table-column>
        </el-table>
      </div>
    </el-dialog>
  </div>
</template>

<script>
import { getProblems } from '@/api/problem';

export default {
  name: 'UserCompetitions',
  props: {
    params: {
      type: Object,
      default: () => ({
        competitions: []
      })
    }
  },
  data() {
    return {
      competitions: [],
      problemsDialogVisible: false,
      currentCompetition: null,
      competitionProblems: [],
      countdownInterval: null
    };
  },
  created() {
    // 从props获取比赛数据
    this.competitions = this.params.competitions || [];
    
    // 设置定时器更新倒计时
    this.countdownInterval = setInterval(() => {
      // 强制更新组件，刷新倒计时显示
      this.$forceUpdate();
    }, 1000);
  },
  beforeDestroy() {
    // 组件销毁前清除定时器
    if (this.countdownInterval) {
      clearInterval(this.countdownInterval);
    }
  },
  methods: {
    // 格式化日期
    formatDate(timestamp) {
      if (!timestamp) return '';
      const date = new Date(timestamp);
      return date.toLocaleString('zh-CN', {
        year: 'numeric',
        month: '2-digit',
        day: '2-digit',
        hour: '2-digit',
        minute: '2-digit'
      });
    },
    
    // 获取倒计时字符串
    getCountdown(endTimestamp) {
      const now = Date.now();
      const timeRemaining = endTimestamp - now;
      
      if (timeRemaining <= 0) {
        return '已结束';
      }
      
      // 计算剩余时间
      const days = Math.floor(timeRemaining / (1000 * 60 * 60 * 24));
      const hours = Math.floor((timeRemaining % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60));
      const minutes = Math.floor((timeRemaining % (1000 * 60 * 60)) / (1000 * 60));
      const seconds = Math.floor((timeRemaining % (1000 * 60)) / 1000);
      
      // 格式化输出
      const parts = [];
      if (days > 0) parts.push(`${days}天`);
      if (hours > 0 || days > 0) parts.push(`${hours}小时`);
      if (minutes > 0 || hours > 0 || days > 0) parts.push(`${minutes}分钟`);
      parts.push(`${seconds}秒`);
      
      return parts.join(' ');
    },
    
    // 获取难度对应的类型
    getDifficultyType(difficulty) {
      const typeMap = {
        '简单': 'success',
        '中等': 'warning',
        '困难': 'danger'
      };
      return typeMap[difficulty] || 'info';
    },
    
    // 查看比赛题目
    async viewCompetitionProblems(competition) {
      this.currentCompetition = competition;
      this.problemsDialogVisible = true;
      
      // 加载比赛题目详情
      if (competition.problems && competition.problems.length > 0) {
        try {
          // 查询所有的题目
          const response = await getProblems({ limit: 1000 });
          if (response.status === 'ok') {
            // 筛选出比赛中包含的题目
            const allProblems = response.problems || [];
            this.competitionProblems = allProblems.filter(problem => 
              competition.problems.includes(problem.id)
            );
          } else {
            this.$message.error(response.message || '获取题目列表失败');
          }
        } catch (error) {
          console.error('获取题目列表失败:', error);
          this.$message.error('获取题目列表失败，请稍后再试');
        }
      } else {
        this.competitionProblems = [];
      }
    },
    
    // 查看题目详情
    viewProblem(problem) {
      // 检查父组件是否有处理方法
      if (this.$parent && typeof this.$parent.handleComponentNavigation === 'function') {
        // 使用父组件的导航方法
        this.$parent.handleComponentNavigation({ 
          component: 'problem-detail', 
          problemId: problem.id 
        });
        // 关闭对话框
        this.problemsDialogVisible = false;
      } else {
        // 备用方案：直接通过路由导航
        this.$router.push({
          path: `/problems/${problem.id}`
        }).catch(err => {
          // 忽略导航重复错误
          if (err.name !== 'NavigationDuplicated') {
            throw err;
          }
        });
      }
    }
  }
}
</script>

<style lang="scss" scoped>
.user-competitions-container {
  padding: 0 0 20px 0;
  
  .competitions-card {
    margin-bottom: 20px;
  }
  
  .card-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
  }
  
  .competition-list {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(300px, 1fr));
    gap: 20px;
    
    @media screen and (max-width: 768px) {
      grid-template-columns: 1fr;
    }
  }
  
  .competition-item {
    transition: transform 0.3s;
    height: 100%;
    display: flex;
    flex-direction: column;
    
    &:hover {
      transform: translateY(-5px);
    }
    
    .competition-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 10px;
      
      .competition-title {
        margin: 0;
        font-size: 18px;
        flex: 1;
        overflow: hidden;
        text-overflow: ellipsis;
        white-space: nowrap;
      }
    }
    
    .competition-description {
      margin: 10px 0;
      color: #606266;
      flex: 1;
      overflow: hidden;
      text-overflow: ellipsis;
      display: -webkit-box;
      -webkit-line-clamp: 3;
      -webkit-box-orient: vertical;
    }
    
    .competition-info {
      margin-top: 10px;
      font-size: 14px;
      color: #909399;
      
      .info-item {
        margin-bottom: 5px;
        display: flex;
        align-items: center;
        
        i {
          margin-right: 5px;
        }
      }
      
      .competition-countdown {
        margin-top: 10px;
        font-weight: bold;
        color: #E6A23C;
        
        .countdown-time {
          color: #F56C6C;
        }
      }
    }
    
    .competition-actions {
      margin-top: 15px;
      display: flex;
      justify-content: flex-end;
    }
  }
  
  .dialog-info {
    margin-bottom: 20px;
    padding: 10px;
    background-color: #f5f7fa;
    border-radius: 4px;
    
    p {
      margin: 5px 0;
    }
    
    .countdown-time {
      color: #F56C6C;
      font-weight: bold;
    }
  }
}
</style> 