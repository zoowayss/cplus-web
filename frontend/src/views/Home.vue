<template>
  <div class="home-container">
    <el-container>
      <el-header>
        <div class="header-left">
          <i class="el-icon-s-fold toggle-menu" @click="isCollapse = !isCollapse"></i>
          <div class="header-logo">管理员系统</div>
        </div>
        <div class="header-right">
          <el-dropdown trigger="click" @command="handleCommand">
            <span class="el-dropdown-link">
              <el-avatar size="small" :src="avatarUrl"></el-avatar>
              {{ username }}<i class="el-icon-arrow-down el-icon--right"></i>
            </span>
            <el-dropdown-menu slot="dropdown">
              <el-dropdown-item command="profile">个人信息</el-dropdown-item>
              <el-dropdown-item command="settings">设置</el-dropdown-item>
              <el-dropdown-item divided command="logout">退出登录</el-dropdown-item>
            </el-dropdown-menu>
          </el-dropdown>
        </div>
      </el-header>
      
      <el-container>
        <el-aside :width="isCollapse ? '64px' : '200px'">
          <el-menu
            :default-active="activeMenu"
            class="el-menu-vertical"
            background-color="#545c64"
            text-color="#fff"
            :collapse="isCollapse"
            active-text-color="#ffd04b"
            @select="handleSelect">
            <el-menu-item index="dashboard">
              <i class="el-icon-s-home"></i>
              <span slot="title">首页</span>
            </el-menu-item>
            <el-submenu index="problem-management">
              <template slot="title">
                <i class="el-icon-s-order"></i>
                <span>题目管理</span>
              </template>
              <el-menu-item index="problem-list">
                <i class="el-icon-tickets"></i>
                <span>题目列表</span>
              </el-menu-item>
              <el-menu-item index="problem-create">
                <i class="el-icon-plus"></i>
                <span>添加题目</span>
              </el-menu-item>
            </el-submenu>
            <el-menu-item index="user-management">
              <i class="el-icon-s-custom"></i>
              <span slot="title">用户管理</span>
            </el-menu-item>
            <el-menu-item index="submission-management">
              <i class="el-icon-s-data"></i>
              <span slot="title">提交记录</span>
            </el-menu-item>
            <el-menu-item index="system-settings">
              <i class="el-icon-s-tools"></i>
              <span slot="title">系统设置</span>
            </el-menu-item>
          </el-menu>
        </el-aside>
        
        <el-main>
          <!-- 根据选中的菜单动态展示内容或使用路由显示 -->
          <router-view v-if="$route.matched.length > 1"></router-view>
          
          <!-- 如果没有匹配的子路由，但有currentComponent，则显示组件 -->
          <component 
            :is="currentComponent" 
            v-else-if="currentComponent" 
            :params="componentParams">
          </component>
          
          <!-- 如果没有匹配的子路由和currentComponent，则显示仪表盘内容 -->
          <div v-else-if="activeMenu === 'dashboard'">
            <el-row :gutter="20">
              <el-col :span="24">
                <el-card class="welcome-card">
                  <div slot="header">
                    <span>在线评测系统管理后台</span>
                  </div>
                  <div class="welcome-content">
                    <h1>欢迎回来，{{ username }}！</h1>
                    <p>您已成功登录管理后台。在这里您可以管理用户、题目、提交记录和系统设置。</p>
                  </div>
                </el-card>
              </el-col>
            </el-row>
            
            <el-row :gutter="20" class="card-row">
              <el-col :xs="24" :sm="24" :md="12" :lg="8" :xl="8" v-for="(card, index) in dashboardCards" :key="index">
                <el-card shadow="hover" :body-style="{ padding: '20px' }">
                  <div class="dashboard-card">
                    <div class="card-icon" :style="{ backgroundColor: card.color }">
                      <i :class="card.icon"></i>
                    </div>
                    <div class="card-content">
                      <h3>{{ card.title }}</h3>
                      <div class="card-number">{{ card.number }}</div>
                      <div class="card-description">{{ card.description }}</div>
                    </div>
                  </div>
                </el-card>
              </el-col>
            </el-row>
            
            <el-row :gutter="20" class="card-row">
              <el-col :xs="24" :sm="24" :md="24" :lg="16" :xl="16">
                <el-card>
                  <div slot="header" class="card-header">
                    <span>最近提交记录</span>
                    <el-button style="padding: 3px 0" type="text" @click="handleSelect('submission-management')">
                      查看全部
                    </el-button>
                  </div>
                  <div class="responsive-table">
                    <el-table :data="recentSubmissions" style="width: 100%">
                      <el-table-column prop="id" label="ID" width="60" :show-overflow-tooltip="true"></el-table-column>
                      <el-table-column prop="problemTitle" label="题目" min-width="120" :show-overflow-tooltip="true"></el-table-column>
                      <el-table-column prop="user" label="用户" width="100" :show-overflow-tooltip="true"></el-table-column>
                      <el-table-column prop="status" label="状态" width="100" :show-overflow-tooltip="true">
                        <template slot-scope="scope">
                          <el-tag :type="getStatusType(scope.row.status)" size="mini">
                            {{ scope.row.status }}
                          </el-tag>
                        </template>
                      </el-table-column>
                      <el-table-column prop="time" label="时间" width="160" :show-overflow-tooltip="true"></el-table-column>
                    </el-table>
                  </div>
                </el-card>
              </el-col>
              
              <el-col :xs="24" :sm="24" :md="24" :lg="8" :xl="8">
                <el-card>
                  <div slot="header" class="card-header">
                    <span>系统通知</span>
                    <el-button style="padding: 3px 0" type="text" @click="handleSelect('notifications')">
                      查看全部
                    </el-button>
                  </div>
                  <div class="notification-list">
                    <div v-for="(notification, index) in notifications" :key="index" class="notification-item">
                      <div class="notification-title">
                        <i :class="notification.icon" :style="{ color: notification.color }"></i>
                        <span>{{ notification.title }}</span>
                      </div>
                      <div class="notification-time">{{ notification.time }}</div>
                      <div class="notification-content">{{ notification.content }}</div>
                    </div>
                    <div v-if="notifications.length === 0" class="empty-data">
                      暂无通知
                    </div>
                  </div>
                </el-card>
              </el-col>
            </el-row>
          </div>
        </el-main>
      </el-container>
    </el-container>
  </div>
</template>

<script>
import { getUserInfo, logout } from '@/api/user'

// 懒加载管理组件
const AdminProblemList = () => import('@/views/admin/problems/ProblemList.vue')
const ProblemEdit = () => import('@/views/admin/problems/ProblemEdit.vue')
const TestCaseManager = () => import('@/views/admin/problems/TestCaseManager.vue')

// 用户管理组件可能需要创建
const UserManagement = {
  template: '<div><h2>用户管理页面</h2><p>此功能正在开发中...</p></div>'
}

// 提交记录组件可能需要创建
const SubmissionManagement = {
  template: '<div><h2>提交记录管理页面</h2><p>此功能正在开发中...</p></div>'
}

// 系统设置组件可能需要创建
const SystemSettings = {
  template: '<div><h2>系统设置页面</h2><p>此功能正在开发中...</p></div>'
}

// 通知组件可能需要创建
const Notifications = {
  template: '<div><h2>系统通知页面</h2><p>此功能正在开发中...</p></div>'
}

export default {
  name: 'Home',
  components: {
    AdminProblemList,
    ProblemEdit,
    TestCaseManager,
    UserManagement,
    SubmissionManagement,
    SystemSettings,
    Notifications
  },
  data() {
    return {
      username: '',
      email: '',
      role: 2, // 默认管理员角色
      roleName: '管理员',
      isCollapse: window.innerWidth < 768,
      activeMenu: 'dashboard',
      currentComponent: null,
      componentParams: {},
      avatarUrl: 'https://cube.elemecdn.com/0/88/03b0d39583f48206768a7534e55bcpng.png',
      
      // 仪表板卡片数据
      dashboardCards: [
        {
          title: '用户总数',
          number: 156,
          description: '系统注册用户总数',
          icon: 'el-icon-user',
          color: '#409EFF'
        },
        {
          title: '题目总数',
          number: 320,
          description: '系统题库题目总数',
          icon: 'el-icon-document',
          color: '#67C23A'
        },
        {
          title: '提交总数',
          number: 1058,
          description: '用户提交代码总数',
          icon: 'el-icon-s-data',
          color: '#E6A23C'
        }
      ],
      
      // 最近提交记录
      recentSubmissions: [
        {
          id: 1001,
          problemTitle: '两数之和',
          user: 'user1',
          status: '通过',
          time: '2023-06-15 15:30:45'
        },
        {
          id: 1002,
          problemTitle: '三数之和',
          user: 'user2',
          status: '失败',
          time: '2023-06-15 14:28:30'
        },
        {
          id: 1003,
          problemTitle: '链表反转',
          user: 'user3',
          status: '通过',
          time: '2023-06-15 13:45:22'
        },
        {
          id: 1004,
          problemTitle: '二叉树遍历',
          user: 'user4',
          status: '编译错误',
          time: '2023-06-15 12:15:10'
        },
        {
          id: 1005,
          problemTitle: '动态规划问题',
          user: 'user5',
          status: '超时',
          time: '2023-06-15 11:30:45'
        }
      ],
      
      // 系统通知
      notifications: [
        {
          title: '系统更新',
          content: '系统已更新到最新版本 v1.2.5，新增了代码高亮和实时评测功能。',
          time: '2023-06-15 09:30',
          icon: 'el-icon-bell',
          color: '#409EFF'
        },
        {
          title: '新增题目',
          content: '管理员已添加10道新题目到题库中，涵盖算法和数据结构相关知识点。',
          time: '2023-06-14 14:20',
          icon: 'el-icon-document',
          color: '#67C23A'
        },
        {
          title: '系统维护',
          content: '系统将于本周日 23:00-24:00 进行例行维护，期间可能无法访问。',
          time: '2023-06-13 16:45',
          icon: 'el-icon-warning',
          color: '#E6A23C'
        }
      ]
    }
  },
  created() {
    // 检查用户是否已登录，若未登录则重定向到登录页
    if (!this.$store.getters.isAuthenticated) {
      this.$message.warning('请先登录')
      this.$router.push('/login')
      return
    }
    
    // 检查用户角色
    const userInfo = this.$store.getters.currentUser
    if (userInfo && userInfo.role < 2) {
      this.$message.warning('权限不足，无法访问管理页面')
      this.$router.push('/user')
      return
    }
    
    // 监听窗口大小变化，调整菜单折叠状态
    window.addEventListener('resize', this.handleResize)
    
    this.fetchUserInfo()
    
    // 检查是否有查询参数，优先处理查询参数
    const queryParams = this.$route.query
    if (queryParams.component) {
      this.handleComponentNavigation(queryParams)
    } else {
      // 如果没有查询参数，则根据URL路径设置初始活动菜单
      this.initActiveMenuFromPath()
    }
  },
  beforeDestroy() {
    // 组件销毁前移除事件监听
    window.removeEventListener('resize', this.handleResize)
  },
  methods: {
    fetchUserInfo() {
      // 优先从store中获取用户信息
      const userInfo = this.$store.getters.currentUser
      if (userInfo) {
        this.username = userInfo.username || '管理员'
        this.email = userInfo.email || '-'
        this.role = userInfo.role || 2
        
        // 如果不是管理员，跳转到用户页面
        if (this.role < 2) {
          this.$message.warning('权限不足，无法访问管理页面')
          this.$router.push('/user')
          return
        }
      } else {
        // 如果store中没有，则重新请求
        getUserInfo()
          .then(response => {
            const { data } = response
            this.username = data.username
            this.email = data.email
            this.role = data.role || 2
            // 更新store中的用户信息
            this.$store.commit('SET_USER', data)
          })
          .catch(error => {
            console.error('获取用户信息失败', error)
            // 如果获取用户信息失败，可能是token无效，跳转到登录页
            this.$store.dispatch('logout')
            this.$message.error('获取用户信息失败，请重新登录')
            this.$router.push('/login')
          })
      }
    },
    handleCommand(command) {
      if (command === 'logout') {
        this.handleLogout()
      } else if (command === 'profile') {
        this.$message.info('个人信息功能开发中')
      } else if (command === 'settings') {
        this.$message.info('设置功能开发中')
      }
    },
    handleLogout() {
      this.$confirm('确认退出登录吗?', '提示', {
        confirmButtonText: '确定',
        cancelButtonText: '取消',
        type: 'warning'
      })
        .then(() => {
          logout()
            .then(() => {
              this.$store.dispatch('logout')
              this.$message.success('退出登录成功')
              this.$router.push('/login')
            })
            .catch(error => {
              console.error('退出登录失败', error)
              // 即使API调用失败，也清除本地登录状态
              this.$store.dispatch('logout')
              this.$router.push('/login')
            })
        })
        .catch(() => {
          this.$message.info('已取消退出')
        })
    },
    handleResize() {
      // 窗口宽度小于768px时折叠菜单
      this.isCollapse = window.innerWidth < 768
    },
    getStatusType(status) {
      const statusMap = {
        '通过': 'success',
        '失败': 'danger',
        '编译错误': 'warning',
        '超时': 'info'
      };
      return statusMap[status] || 'info';
    },
    handleSelect(key) {
      this.activeMenu = key;
      
      switch (key) {
        case 'dashboard':
          this.$router.push('/admin');
          break;
        case 'problem-list':
          this.$router.push('/admin/problems');
          break;
        case 'problem-create':
          this.$router.push('/admin/problems/create');
          break;
        case 'user-management':
          this.$router.push('/admin/users');
          break;
        case 'submission-management':
          this.$router.push('/admin/submissions');
          break;
        case 'system-settings':
          this.$router.push('/admin/settings');
          break;
        default:
          this.$router.push('/admin');
      }
    },
    
    editProblem(id) {
      this.currentComponent = 'ProblemEdit';
      this.componentParams = { id, isCreate: false };
      this.activeMenu = 'problem-edit';
      
      // 更新浏览器历史，使用 router.push 替代 window.history.pushState
      this.$router.push({
        path: `/admin/problems/edit/${id}`,
        query: { fromAdmin: true } // 添加标记，表示这是从管理界面发起的导航
      }).catch(err => {
        // 忽略导航重复错误
        if (err.name !== 'NavigationDuplicated') {
          throw err;
        }
      });
    },
    
    manageProblemTestCases(id) {
      // 设置当前组件为TestCaseManager
      this.currentComponent = 'TestCaseManager';
      this.componentParams = { id };
      this.activeMenu = 'problem-testcases';
      
      // 更新浏览器历史，使用 router.push 替代 window.history.pushState
      this.$router.push({
        path: `/admin/problems/testcases/${id}`,
        query: { fromAdmin: true } // 添加标记，表示这是从管理界面发起的导航
      }).catch(err => {
        // 忽略导航重复错误
        if (err.name !== 'NavigationDuplicated') {
          throw err;
        }
      });
      
      // 确保组件会重新渲染
      this.$nextTick(() => {
        console.log('准备加载测试用例管理组件，ID:', id);
        // 如果需要，可以在这里添加额外的处理逻辑
      });
    },
    
    initActiveMenuFromPath() {
      const path = window.location.pathname;
      
      if (path.includes('/admin/problems/edit')) {
        const id = parseInt(path.split('/').pop());
        if (!isNaN(id)) {
          this.editProblem(id);
        }
      } else if (path.includes('/admin/problems/create')) {
        this.handleSelect('problem-create');
      } else if (path.includes('/admin/problems/testcases')) {
        const id = parseInt(path.split('/').pop());
        if (!isNaN(id)) {
          this.manageProblemTestCases(id);
        }
      } else if (path.includes('/admin/problems')) {
        this.handleSelect('problem-list');
      } else if (path.includes('/admin/users')) {
        this.handleSelect('user-management');
      } else if (path.includes('/admin/submissions')) {
        this.handleSelect('submission-management');
      } else if (path.includes('/admin/settings')) {
        this.handleSelect('system-settings');
      } else {
        this.handleSelect('dashboard');
      }
    },
    
    // 处理URL查询参数导航
    handleComponentNavigation(params) {
      const component = params.component
      
      switch (component) {
        case 'problem-detail':
          if (params.problemId) {
            const problemId = parseInt(params.problemId)
            this.currentComponent = 'ProblemEdit'
            this.componentParams = { id: problemId, isCreate: false }
            this.activeMenu = 'problem-edit'
            
            // 更新URL（可选，但建议添加，以支持刷新和分享）
            this.$router.replace({
              path: this.$route.path,
              query: { component, problemId }
            }).catch(err => {
              // 忽略导航重复错误
              if (err.name !== 'NavigationDuplicated') {
                throw err;
              }
            });
          } else {
            this.handleSelect('problem-list')
          }
          break
        case 'problems':
          this.handleSelect('problem-list')
          break
        case 'submissions':
          this.handleSelect('submission-management')
          break
        case 'leaderboard': // 如果需要添加排行榜组件
          // TODO: 添加排行榜组件处理
          this.handleSelect('dashboard')
          break
        default:
          this.handleSelect('dashboard')
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
      
      .toggle-menu {
        font-size: 20px;
        margin-right: 15px;
        cursor: pointer;
        transition: transform 0.3s;
        
        &:hover {
          transform: scale(1.1);
        }
      }
      
      .header-logo {
        font-size: 20px;
        font-weight: bold;
        white-space: nowrap;
        
        @media screen and (max-width: 576px) {
          font-size: 16px;
        }
      }
    }
    
    .header-right {
      .el-dropdown-link {
        color: #fff;
        cursor: pointer;
        display: flex;
        align-items: center;
        
        .el-avatar {
          margin-right: 10px;
        }
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
    
    .welcome-card {
      margin-bottom: 20px;
      
      .welcome-content {
        padding: 20px;
        text-align: center;
        
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
    }
    
    .card-row {
      margin-top: 20px;
      
      @media screen and (max-width: 576px) {
        margin-top: 10px;
      }
      
      .el-col {
        margin-bottom: 20px;
        
        @media screen and (max-width: 576px) {
          margin-bottom: 10px;
        }
      }
      
      .card-item {
        p {
          margin: 10px 0;
          
          @media screen and (max-width: 576px) {
            margin: 5px 0;
            font-size: 12px;
          }
        }
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
  
  .welcome-card .welcome-content h1 {
    font-size: 18px;
  }
}

.dashboard-card {
  display: flex;
  align-items: center;
  
  @media screen and (max-width: 576px) {
    flex-direction: column;
    text-align: center;
  }
  
  .card-icon {
    width: 64px;
    height: 64px;
    border-radius: 8px;
    display: flex;
    align-items: center;
    justify-content: center;
    margin-right: 16px;
    
    @media screen and (max-width: 576px) {
      margin-right: 0;
      margin-bottom: 16px;
    }
    
    i {
      font-size: 28px;
      color: white;
    }
  }
  
  .card-content {
    flex: 1;
    
    h3 {
      margin: 0 0 8px 0;
      font-size: 16px;
      color: #606266;
    }
    
    .card-number {
      font-size: 24px;
      font-weight: bold;
      color: #303133;
      margin-bottom: 4px;
    }
    
    .card-description {
      color: #909399;
      font-size: 13px;
    }
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

.notification-list {
  max-height: 400px;
  overflow-y: auto;
  
  .notification-item {
    padding: 10px 0;
    border-bottom: 1px solid #EBEEF5;
    
    &:last-child {
      border-bottom: none;
    }
    
    .notification-title {
      display: flex;
      align-items: center;
      margin-bottom: 5px;
      
      i {
        margin-right: 5px;
        font-size: 16px;
      }
      
      span {
        font-weight: bold;
        font-size: 14px;
      }
    }
    
    .notification-time {
      font-size: 12px;
      color: #909399;
      margin-bottom: 5px;
    }
    
    .notification-content {
      font-size: 13px;
      color: #606266;
      line-height: 1.5;
    }
  }
  
  .empty-data {
    text-align: center;
    padding: 20px;
    color: #909399;
    font-size: 14px;
  }
}

@media screen and (max-width: 768px) {
  .card-row {
    margin-bottom: 10px;
  }
  
  .el-card {
    margin-bottom: 15px;
  }
}
</style> 