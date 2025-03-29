<template>
  <div class="login-container">
    <div class="login-card">
      <h2 class="login-title">用户登录</h2>
      
      <el-form 
        ref="loginForm"
        :model="loginForm"
        :rules="rules"
        label-position="top"
        @keyup.enter="handleLogin"
      >
        <el-form-item label="用户名" prop="username">
          <el-input 
            v-model="loginForm.username" 
            placeholder="请输入用户名"
            prefix-icon="User"
          />
        </el-form-item>
        
        <el-form-item label="密码" prop="password">
          <el-input 
            v-model="loginForm.password" 
            type="password" 
            placeholder="请输入至少6位字符密码" 
            show-password
            prefix-icon="Lock"
          />
        </el-form-item>
        
        <el-form-item>
          <el-button 
            type="primary" 
            @click="handleLogin" 
            :loading="loading"
            class="login-button"
          >
            登录
          </el-button>
        </el-form-item>
        
        <div class="login-footer">
          <span>没有账号？</span>
          <el-button type="text" @click="goToRegister">立即注册</el-button>
        </div>
      </el-form>
    </div>
  </div>
</template>

<script>
import { login } from '@/api/user'
import { getUserInfo } from '@/api/user'

export default {
  name: 'Login',
  data() {
    return {
      // 登录表单数据
      loginForm: {
        username: '',
        password: ''
      },
      // 表单验证规则
      rules: {
        username: [
          { required: true, message: '请输入用户名', trigger: 'blur' },
          { min: 3, max: 20, message: '用户名长度应在3-20个字符之间', trigger: 'blur' }
        ],
        password: [
          { required: true, message: '请输入密码', trigger: 'blur' },
          { min: 6, message: '密码不能少于6位字符', trigger: 'blur' }
        ]
      },
      // 登录状态
      loading: false
    }
  },
  methods: {
    // 保存用户信息到本地存储
    saveUserData(token) {
      // 保存token
      this.$store.commit('SET_TOKEN', token)
      this.$message.success('登录成功')
      
      // 获取并保存用户信息
      this.fetchUserInfo()
    },
    
    // 获取用户信息
    fetchUserInfo() {
      getUserInfo()
        .then(response => {
          console.log('获取用户信息成功:', response);
          // 保存用户信息到store
          this.$store.commit('SET_USER', response.user)
          
          // 根据用户角色重定向到对应的首页
          const user = response.user || {}
          const isAdmin = user.role >= 2 // 角色值>=2为管理员
          
          // 跳转到对应的首页
          const redirectPath = this.$route.query.redirect || (isAdmin ? '/admin' : '/user')
          this.$router.replace(redirectPath)
        })
        .catch(error => {
          console.error('获取用户信息失败:', error)
          this.$message.error('获取用户信息失败，请重新登录')
          this.$store.dispatch('logout')
        })
    },
    
    // 登录方法
    handleLogin() {
      this.$refs.loginForm.validate(valid => {
        if (valid) {
          this.loading = true
          login(this.loginForm)
            .then(response => {
              if (response.status === 'ok') {
                // 登录成功，保存token并获取用户信息
                this.saveUserData(response.token)
              } else {
                this.$message.error(response.message || '登录失败')
              }
            })
            .catch(error => {
              console.error('登录失败:', error)
              this.$message.error(error.response?.data?.message || '登录失败，请检查用户名和密码')
            })
            .finally(() => {
              this.loading = false
            })
        } else {
          this.$message.warning('请正确填写登录信息')
          return false
        }
      })
    },
    
    // 跳转到注册页面
    goToRegister() {
      this.$router.push('/register')
    }
  }
}
</script>

<style lang="scss" scoped>
.login-container {
  display: flex;
  justify-content: center;
  align-items: center;
  height: 100vh;
  padding: 40px 20px;
  background-color: #f3f3f3;
}

.login-card {
  width: 100%;
  max-width: 400px;
  padding: 30px;
  background-color: #fff;
  border-radius: 8px;
  box-shadow: 0 2px 12px rgba(0, 0, 0, 0.1);
}

.login-title {
  margin-bottom: 30px;
  text-align: center;
  font-size: 24px;
  color: #303133;
}

.login-button {
  width: 100%;
  margin-top: 10px;
}

.login-footer {
  margin-top: 20px;
  text-align: center;
  color: #606266;
}

.login-footer .el-button {
  padding: 0 5px;
  margin-left: 5px;
}
</style> 