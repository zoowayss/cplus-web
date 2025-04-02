<template>
  <el-container class="home-container">
    <el-header class="header">
      <div class="header-left">
        <div class="toggle-btn" @click="isCollapse = !isCollapse">
          <i :class="isCollapse ? 'el-icon-s-unfold' : 'el-icon-s-fold'"></i>
        </div>
        <div class="logo">评测系统</div>
      </div>
      <div class="user-info">
        <el-dropdown @command="handleCommand">
          <span class="el-dropdown-link">
            {{ username }}<i class="el-icon-arrow-down el-icon--right"></i>
          </span>
          <el-dropdown-menu slot="dropdown">
            <el-dropdown-item command="profile">个人资料</el-dropdown-item>
            <el-dropdown-item command="settings">设置</el-dropdown-item>
            <el-dropdown-item divided command="logout">退出登录</el-dropdown-item>
          </el-dropdown-menu>
        </el-dropdown>
      </div>
    </el-header>
    <el-container class="main-container">
      <el-aside :width="isCollapse ? '64px' : '200px'" class="aside">
        <el-menu
          :default-active="activeMenu"
          class="el-menu-vertical"
          @select="handleSelect"
          :collapse="isCollapse"
          background-color="#304156"
          text-color="#bfcbd9"
          active-text-color="#409EFF"
          :unique-opened="true"
          :router="false"
        >
          <el-menu-item index="dashboard">
            <i class="el-icon-s-home"></i>
            <span slot="title">首页</span>
          </el-menu-item>
          <el-menu-item index="problems">
            <i class="el-icon-s-order"></i>
            <span slot="title">题目列表</span>
          </el-menu-item>
          <el-menu-item index="competitions">
            <i class="el-icon-trophy"></i>
            <span slot="title">比赛</span>
          </el-menu-item>
          <el-menu-item index="submissions">
            <i class="el-icon-s-data"></i>
            <span slot="title">提交记录</span>
          </el-menu-item>
          <el-menu-item index="leaderboard">
            <i class="el-icon-trophy"></i>
            <span slot="title">排行榜</span>
          </el-menu-item>
        </el-menu>
      </el-aside>
      <el-main class="main-content">
        <component :is="currentComponent" v-if="currentComponent" :params="componentParams"></component>
        <div v-else class="dashboard-container">
          <el-row :gutter="20">
            <el-col :span="24">
              <div class="welcome-message">
                <h1>欢迎, {{ username }}</h1>
                <p>这是您的个人评测系统控制台</p>
              </div>
            </el-col>
          </el-row>
          <el-row :gutter="20">
            <el-col :xs="24" :sm="12" :md="8" :lg="6">
              <el-card shadow="hover" class="dashboard-card">
                <div class="card-title">
                  <i class="el-icon-s-order"></i>
                  <span>做题进度</span>
                </div>
                <div class="card-content">
                  <div class="stat-number">{{ solvedCount }}</div>
                  <div class="stat-desc">已解决题目</div>
                </div>
              </el-card>
            </el-col>
            <el-col :xs="24" :sm="12" :md="8" :lg="6">
              <el-card shadow="hover" class="dashboard-card">
                <div class="card-title">
                  <i class="el-icon-s-claim"></i>
                  <span>提交情况</span>
                </div>
                <div class="card-content">
                  <div class="stat-number">{{ submissionCount }}</div>
                  <div class="stat-desc">总提交次数</div>
                </div>
              </el-card>
            </el-col>
            <el-col :xs="24" :sm="12" :md="8" :lg="6">
              <el-card shadow="hover" class="dashboard-card">
                <div class="card-title">
                  <i class="el-icon-pie-chart"></i>
                  <span>通过率</span>
                </div>
                <div class="card-content">
                  <div class="stat-number">{{ passRate }}%</div>
                  <div class="stat-desc">代码通过率</div>
                </div>
              </el-card>
            </el-col>
            <el-col :xs="24" :sm="12" :md="8" :lg="6">
              <el-card shadow="hover" class="dashboard-card">
                <div class="card-title">
                  <i class="el-icon-trophy"></i>
                  <span>排名</span>
                </div>
                <div class="card-content">
                  <div class="stat-number">{{ rank }}</div>
                  <div class="stat-desc">当前排名</div>
                </div>
              </el-card>
            </el-col>
          </el-row>
          <el-row :gutter="20" class="recent-activity-row">
            <el-col :span="24">
              <el-card shadow="hover" class="recent-activity-card">
                <div slot="header" class="card-header">
                  <span>最近活动</span>
                </div>
                <div v-if="recentActivities.length > 0">
                  <el-timeline>
                    <el-timeline-item
                      v-for="(activity, index) in recentActivities"
                      :key="index"
                      :timestamp="activity.time"
                      :type="activity.type"
                    >
                      {{ activity.content }}
                    </el-timeline-item>
                  </el-timeline>
                </div>
                <div v-else class="no-data">
                  <i class="el-icon-info"></i>
                  <span>暂无活动记录</span>
                </div>
              </el-card>
            </el-col>
          </el-row>
        </div>
      </el-main>
    </el-container>
  </el-container>
</template>

<script>
import { getUserInfo, logout } from '@/api/user'
// 导入用户比赛组件
const UserCompetitions = () => import('@/views/competitions/UserCompetitions.vue')

// 懒加载组件
const ProblemList = () => import('@/views/problems/ProblemList.vue')
const ProblemDetail = () => import('@/views/problems/ProblemDetail.vue')
const SubmissionList = () => import('@/views/problems/SubmissionList.vue')
const Leaderboard = () => import('@/views/problems/Leaderboard.vue')

// 临时创建账户设置组件
const AccountSettings = {
  template: '<div><h2>账户设置</h2><p>账户设置功能正在开发中...</p></div>'
}

export default {
  name: 'UserHome',
  components: {
    UserCompetitions,
    ProblemList,
    ProblemDetail,
    SubmissionList,
    Leaderboard,
    AccountSettings
  },
  data() {
    return {
      username: this.$store.state.user ? this.$store.state.user.username : '用户',
      email: '',
      role: 1, // 默认普通用户角色
      isCollapse: window.innerWidth < 768,
      solvedCount: 25,
      submissionCount: 87,
      passRate: 68,
      rank: 42,
      recentActivities: [
        {
          content: '完成了题目 "字符串匹配算法"',
          time: '2023-11-12 14:30',
          type: 'success'
        },
        {
          content: '提交了题目 "动态规划问题"',
          time: '2023-11-10 09:15',
          type: 'warning'
        },
        {
          content: '注册成为新用户',
          time: '2023-11-01 10:00',
          type: 'info'
        }
      ],
      activeMenu: 'dashboard',
      currentComponent: null,
      componentParams: {},
      // 添加比赛相关数据
      competitions: [],
    }
  },
  created() {
    // 登录状态检查
    if (!this.$store.getters.isAuthenticated) {
      this.$message.warning('请先登录');
      this.$router.push('/login');
      return;
    }
    
    // 加载用户信息
    this.fetchUserInfo();
    
    // 加载比赛数据
    this.loadCompetitions();
    
    // 监听窗口大小变化
    window.addEventListener('resize', this.handleResize);
    
    // 检查URL查询参数
    const queryParams = this.$route.query;
    if (queryParams.component) {
      this.handleComponentNavigation(queryParams);
    }
  },
  mounted() {
    // 监听URL查询参数变化，实时更新组件
    this.$watch(
      '$route.query', 
      (newQuery) => {
        if (newQuery.component) {
          this.handleComponentNavigation(newQuery)
        }
      }
    )
  },
  beforeDestroy() {
    window.removeEventListener('resize', this.handleResize)
  },
  methods: {
    handleResize() {
      this.isCollapse = window.innerWidth <= 768
    },
    handleCommand(command) {
      if (command === 'logout') {
        this.$store.dispatch('logout')
        this.$router.push('/login')
      } else if (command === 'profile') {
        // 跳转到个人资料页
        this.$message.info('个人资料功能开发中')
      } else if (command === 'settings') {
        // 跳转到设置页
        this.$message.info('设置功能开发中')
      }
    },
    handleSelect(key) {
      this.activeMenu = key;
      
      switch (key) {
        case 'dashboard':
          this.currentComponent = null;
          this.$router.push('/user');
          break;
        case 'problems':
          this.currentComponent = 'ProblemList';
          this.componentParams = { isAdmin: false };
          break;
        case 'competitions':
          this.currentComponent = 'UserCompetitions';
          this.componentParams = { competitions: this.competitions };
          break;
        case 'submissions':
          this.currentComponent = 'SubmissionList';
          this.componentParams = { isAdmin: false };
          break;
        case 'leaderboard':
          this.currentComponent = 'Leaderboard';
          break;
        case 'account':
          this.currentComponent = 'AccountSettings';
          break;
        default:
          this.currentComponent = null;
      }
      
      // 更新URL以支持刷新和分享
      const query = { component: key }
      if (Object.keys(this.componentParams).length > 0) {
        Object.assign(query, this.componentParams)
      }
      
      this.$router.replace({ 
        path: this.$route.path,
        query: query
      })
    },
    // 跳转到题目详情页
    viewProblem(problemId) {
      this.currentComponent = 'ProblemDetail'
      this.componentParams = { problemId }
      this.activeMenu = 'problems' // 高亮题目菜单
      
      // 更新URL
      this.$router.replace({ 
        path: this.$route.path,
        query: { 
          component: 'problem-detail',
          problemId 
        }
      })
    },
    // 跳转到提交详情页
    viewSubmission(submissionId) {
      this.currentComponent = 'SubmissionList'
      this.componentParams = { submissionId }
      this.activeMenu = 'submissions' // 高亮提交菜单
      
      // 更新URL
      this.$router.replace({ 
        path: this.$route.path,
        query: { 
          component: 'submissions', 
          submissionId 
        }
      })
    },
    // 处理URL参数导航
    handleComponentNavigation(queryParams) {
      // 根据URL参数加载相应组件
      const componentName = queryParams.component
      
      switch(componentName) {
        case 'problems':
          this.activeMenu = 'problems'
          this.currentComponent = 'ProblemList'
          this.componentParams = {}
          break
        case 'problem-detail':
          this.activeMenu = 'problems'
          this.currentComponent = 'ProblemDetail'
          this.componentParams = { problemId: queryParams.problemId }
          break
        case 'competitions':
          this.activeMenu = 'competitions'
          this.currentComponent = 'UserCompetitions'
          this.componentParams = { competitions: this.competitions }
          break
        case 'submissions':
          this.activeMenu = 'submissions'
          this.currentComponent = 'SubmissionList'
          this.componentParams = {}
          break
        case 'leaderboard':
          this.activeMenu = 'leaderboard'
          this.currentComponent = 'Leaderboard'
          this.componentParams = {}
          break
        default:
          this.activeMenu = 'dashboard'
          this.currentComponent = null
          this.componentParams = {}
      }
    },
    // 加载比赛数据
    loadCompetitions() {
      try {
        const competitionsJson = localStorage.getItem('competitions');
        if (competitionsJson) {
          let allCompetitions = JSON.parse(competitionsJson);
          // 过滤掉未开始和已结束的比赛
          const now = Date.now();
          this.competitions = allCompetitions.filter(comp => 
            comp.startDate <= now && comp.endDate >= now
          );
        } else {
          this.competitions = [];
        }
      } catch (error) {
        console.error('加载比赛数据失败:', error);
        this.competitions = [];
      }
    },
    
    // 加载用户信息
    fetchUserInfo() {
      // 优先从store中获取用户信息
      const userInfo = this.$store.getters.currentUser;
      if (userInfo) {
        this.username = userInfo.username || '用户';
        this.email = userInfo.email || '';
        this.role = userInfo.role || 1;
      } else {
        // 如果store中没有，则调用API获取
        getUserInfo()
          .then(response => {
            const { data } = response;
            this.username = data.username || '用户';
            this.email = data.email || '';
            this.role = data.role || 1;
            // 更新store中的用户信息
            this.$store.commit('SET_USER', data);
          })
          .catch(error => {
            console.error('获取用户信息失败', error);
            this.$message.error('获取用户信息失败，请重新登录');
            // 登录状态失效，返回登录页
            this.$store.dispatch('logout');
            this.$router.push('/login');
          });
      }
    }
  }
}
</script>

<style lang="scss" scoped>
.home-container {
  height: 100vh;
  display: flex;
  overflow: hidden;
  
  .el-header {
    background-color: #409EFF;
    color: #fff;
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 0 20px;
    height: 60px !important;
    
    .header-left {
      display: flex;
      align-items: center;
      
      .toggle-btn {
        font-size: 20px;
        margin-right: 15px;
        cursor: pointer;
        transition: transform 0.3s;
        
        &:hover {
          transform: scale(1.1);
        }
      }
      
      .logo {
        font-size: 20px;
        font-weight: bold;
        white-space: nowrap;
        
        @media screen and (max-width: 576px) {
          font-size: 16px;
        }
      }
    }
    
    .user-info {
      .el-dropdown-link {
        color: #fff;
        cursor: pointer;
        display: flex;
        align-items: center;
      }
    }
  }
  
  .el-aside {
    background-color: #545c64;
    color: #fff;
    transition: width 0.3s;
    overflow: hidden;
    
    @media screen and (max-width: 768px) {
      width: 64px !important;
    }
    
    .el-menu {
      border-right: none;
    }
  }
  
  .el-main {
    background-color: #f0f2f5;
    padding: 20px;
    overflow: auto;
    
    @media screen and (max-width: 768px) {
      padding: 10px;
    }
    
    .welcome-message {
      margin-bottom: 20px;
      
      h1 {
        margin-top: 0;
        font-size: calc(1.2rem + 1vw);
      }
      
      p {
        font-size: 14px;
        
        @media screen and (max-width: 576px) {
          font-size: 12px;
        }
      }
    }
    
    .dashboard-container {
      .dashboard-card {
        margin-bottom: 20px;
        
        @media screen and (max-width: 576px) {
          margin-bottom: 10px;
        }
        
        .card-title {
          display: flex;
          justify-content: space-between;
          align-items: center;
          flex-wrap: wrap;
          
          @media screen and (max-width: 576px) {
            font-size: 14px;
          }
        }
        
        .card-content {
          padding: 20px;
          text-align: center;
          
          .stat-number {
            font-size: 24px;
            font-weight: bold;
            margin-bottom: 10px;
          }
          
          .stat-desc {
            font-size: 14px;
            
            @media screen and (max-width: 576px) {
              font-size: 12px;
            }
          }
        }
      }
    }
    
    .recent-activity-row {
      margin-top: 20px;
      
      @media screen and (max-width: 576px) {
        margin-top: 10px;
      }
    }
  }
  
  // 使el-container自适应高度
  .el-container {
    height: 100%;
    width: 100%;
    
    &:nth-child(2) {
      overflow: hidden;
    }
  }
}

// 增加媒体查询，处理小屏幕设备的布局调整
@media screen and (max-width: 768px) {
  .el-col-12 {
    width: 100%;
  }
  
  .el-menu-item span {
    display: none;
  }
  
  .welcome-message h1 {
    font-size: 18px;
  }
}

.responsive-table {
  overflow-x: auto;
  
  .el-table {
    width: 100%;
    
    @media screen and (max-width: 576px) {
      font-size: 12px;
    }
  }
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  flex-wrap: wrap;
  
  @media screen and (max-width: 576px) {
    font-size: 14px;
  }
}
</style> 