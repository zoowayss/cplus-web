<template>
  <div class="home-container">
    <el-container>
      <el-header>
        <div class="header-logo">Cplus Web</div>
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
              <i class="el-icon-document"></i>
              <span slot="title">内容管理</span>
            </el-menu-item>
            <el-menu-item index="3">
              <i class="el-icon-setting"></i>
              <span slot="title">系统设置</span>
            </el-menu-item>
          </el-menu>
        </el-aside>
        
        <el-main>
          <el-card class="welcome-card">
            <div slot="header">
              <span>欢迎使用</span>
            </div>
            <div class="welcome-content">
              <h1>欢迎回来，{{ username }}！</h1>
              <p>您已成功登录系统。</p>
              <p>这是系统首页，您可以从左侧菜单选择功能。</p>
            </div>
          </el-card>
          
          <el-row :gutter="20" class="card-row">
            <el-col :span="12">
              <el-card shadow="hover">
                <div slot="header">
                  <i class="el-icon-user"></i>
                  <span>用户信息</span>
                </div>
                <div class="card-item">
                  <p><strong>用户名：</strong> {{ username }}</p>
                  <p><strong>邮箱：</strong> {{ email }}</p>
                  <p><strong>角色：</strong> {{ roleName }}</p>
                </div>
              </el-card>
            </el-col>
            <el-col :span="12">
              <el-card shadow="hover">
                <div slot="header">
                  <i class="el-icon-bell"></i>
                  <span>通知公告</span>
                </div>
                <div class="card-item">
                  <p>暂无通知</p>
                </div>
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
  name: 'Home',
  data() {
    return {
      username: '',
      email: '',
      role: 2, // 默认管理员角色
      roleName: '管理员',
      avatarUrl: 'https://cube.elemecdn.com/0/88/03b0d39583f48206768a7534e55bcpng.png'
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
    
    this.fetchUserInfo()
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