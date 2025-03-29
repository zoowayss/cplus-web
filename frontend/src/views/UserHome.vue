<template>
  <div class="home-container">
    <el-container>
      <el-header>
        <div class="header-logo">在线评测系统</div>
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
        <el-aside width="200px">
          <el-menu
            default-active="1"
            class="el-menu-vertical"
            background-color="#545c64"
            text-color="#fff"
            active-text-color="#ffd04b">
            <el-menu-item index="1">
              <i class="el-icon-s-home"></i>
              <span slot="title">首页</span>
            </el-menu-item>
            <el-menu-item index="2">
              <i class="el-icon-s-order"></i>
              <span slot="title">题目列表</span>
            </el-menu-item>
            <el-menu-item index="3">
              <i class="el-icon-s-promotion"></i>
              <span slot="title">我的提交</span>
            </el-menu-item>
            <el-menu-item index="4">
              <i class="el-icon-s-data"></i>
              <span slot="title">排行榜</span>
            </el-menu-item>
          </el-menu>
        </el-aside>
        
        <el-main>
          <el-row :gutter="20">
            <el-col :span="24">
              <el-card class="welcome-card">
                <div slot="header">
                  <span>欢迎使用在线评测系统</span>
                </div>
                <div class="welcome-content">
                  <h1>欢迎回来，{{ username }}！</h1>
                  <p>您已成功登录系统。这是学生用户界面，您可以在这里参与题目的练习和提交。</p>
                </div>
              </el-card>
            </el-col>
          </el-row>
          
          <el-row :gutter="20" class="card-row">
            <el-col :span="12">
              <el-card shadow="hover">
                <div slot="header">
                  <i class="el-icon-user"></i>
                  <span>个人信息</span>
                </div>
                <div class="card-item">
                  <p><strong>用户名：</strong> {{ username }}</p>
                  <p><strong>邮箱：</strong> {{ email }}</p>
                  <p><strong>身份：</strong> {{ roleName }}</p>
                </div>
              </el-card>
            </el-col>
            <el-col :span="12">
              <el-card shadow="hover">
                <div slot="header">
                  <i class="el-icon-data-analysis"></i>
                  <span>统计信息</span>
                </div>
                <div class="card-item">
                  <p><strong>题目总数：</strong> {{ stats.totalProblems }}</p>
                  <p><strong>已提交：</strong> {{ stats.totalSubmissions }}</p>
                  <p><strong>已解决：</strong> {{ stats.solvedProblems }}</p>
                </div>
              </el-card>
            </el-col>
          </el-row>
          
          <el-row :gutter="20" class="card-row">
            <el-col :span="24">
              <el-card>
                <div slot="header">
                  <span>推荐题目</span>
                  <el-button style="float: right; padding: 3px 0" type="text" @click="viewAllProblems">
                    查看全部
                  </el-button>
                </div>
                <el-table :data="recommendedProblems" style="width: 100%">
                  <el-table-column prop="id" label="ID" width="80"></el-table-column>
                  <el-table-column prop="title" label="题目" min-width="180"></el-table-column>
                  <el-table-column prop="difficulty" label="难度" width="100">
                    <template slot-scope="scope">
                      <el-tag :type="getDifficultyType(scope.row.difficulty)">
                        {{ scope.row.difficulty }}
                      </el-tag>
                    </template>
                  </el-table-column>
                  <el-table-column prop="acceptance" label="通过率" width="100"></el-table-column>
                  <el-table-column label="操作" width="120">
                    <template slot-scope="scope">
                      <el-button type="text" size="small" @click="goToProblem(scope.row.id)">
                        查看详情
                      </el-button>
                    </template>
                  </el-table-column>
                </el-table>
              </el-card>
            </el-col>
          </el-row>
        </el-main>
      </el-container>
    </el-container>
  </div>
</template>

<script>
import { getUserInfo, logout } from '@/api/user'

export default {
  name: 'UserHome',
  data() {
    return {
      username: '',
      email: '',
      role: 0, // 0: 学生, 1: 教师, 2: 管理员
      avatarUrl: 'https://cube.elemecdn.com/0/88/03b0d39583f48206768a7534e55bcpng.png',
      stats: {
        totalProblems: 0,
        totalSubmissions: 0,
        solvedProblems: 0
      },
      recommendedProblems: [
        { id: 1, title: '两数之和', difficulty: '简单', acceptance: '75%' },
        { id: 2, title: '链表反转', difficulty: '中等', acceptance: '62%' },
        { id: 3, title: '最长回文子串', difficulty: '中等', acceptance: '45%' },
        { id: 4, title: '合并区间', difficulty: '中等', acceptance: '40%' },
        { id: 5, title: '最小路径和', difficulty: '困难', acceptance: '32%' }
      ]
    }
  },
  computed: {
    roleName() {
      const roles = ['学生', '教师', '管理员'];
      return roles[this.role] || '学生';
    }
  },
  created() {
    // 检查用户是否已登录，若未登录则重定向到登录页
    if (!this.$store.getters.isAuthenticated) {
      this.$message.warning('请先登录')
      this.$router.push('/login')
      return
    }
    this.fetchUserInfo()
  },
  methods: {
    fetchUserInfo() {
      // 优先从store中获取用户信息
      const userInfo = this.$store.getters.currentUser
      if (userInfo) {
        this.username = userInfo.username || '用户'
        this.email = userInfo.email || '-'
        this.role = userInfo.role || 0
        
        // 这里可以根据用户角色判断是否需要重定向到管理员页面
        if (this.role >= 2) { // 管理员
          this.$router.replace('/admin')
        }
      } else {
        // 如果store中没有，则重新请求
        getUserInfo()
          .then(response => {
            const { user } = response.user || {}
            this.username = user.username || '用户'
            this.email = user.email || '-'
            this.role = user.role || 0
            
            // 更新store中的用户信息
            this.$store.commit('SET_USER', user)
            
            // 这里可以根据用户角色判断是否需要重定向到管理员页面
            if (this.role >= 2) { // 管理员
              this.$router.replace('/admin')
            }
          })
          .catch(error => {
            console.error('获取用户信息失败:', error)
            // 如果获取用户信息失败，可能是token无效，跳转到登录页
            this.$store.dispatch('logout')
            this.$message.error('获取用户信息失败，请重新登录')
            this.$router.push('/login')
          })
      }
      
      // 模拟获取统计数据
      this.fetchUserStats()
    },
    fetchUserStats() {
      // 这里应该是API请求，目前使用模拟数据
      this.stats = {
        totalProblems: 150,
        totalSubmissions: 27,
        solvedProblems: 15
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
              console.error('退出登录失败:', error)
              // 即使API调用失败，也清除本地登录状态
              this.$store.dispatch('logout')
              this.$router.push('/login')
            })
        })
        .catch(() => {
          this.$message.info('已取消退出')
        })
    },
    getDifficultyType(difficulty) {
      const types = {
        '简单': 'success',
        '中等': 'warning',
        '困难': 'danger'
      }
      return types[difficulty] || 'info'
    },
    viewAllProblems() {
      this.$message.info('题目列表功能开发中')
    },
    goToProblem(id) {
      this.$message.info(`题目详情功能开发中，题目ID: ${id}`)
    }
  }
}
</script>

<style lang="scss" scoped>
.home-container {
  height: 100vh;
  
  .el-header {
    background-color: #409EFF;
    color: #fff;
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 0 20px;
    
    .header-logo {
      font-size: 20px;
      font-weight: bold;
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
    
    .el-menu {
      border-right: none;
    }
  }
  
  .el-main {
    background-color: #f0f2f5;
    padding: 20px;
    overflow: auto;
    
    .welcome-card {
      margin-bottom: 20px;
      
      .welcome-content {
        padding: 20px;
        text-align: center;
        
        h1 {
          margin-top: 0;
        }
      }
    }
    
    .card-row {
      margin-top: 20px;
      
      .card-item {
        p {
          margin: 10px 0;
        }
      }
    }
  }
}
</style> 