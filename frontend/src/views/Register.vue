<template>
  <div class="register-container">
    <el-card class="register-card">
      <div class="title-container">
        <h3 class="title">用户注册</h3>
      </div>

      <el-form ref="registerForm" :model="registerForm" :rules="registerRules" class="register-form" autocomplete="on" label-position="left">
        <el-form-item prop="username">
          <el-input
            ref="username"
            v-model="registerForm.username"
            placeholder="用户名"
            name="username"
            type="text"
            autocomplete="on"
            prefix-icon="el-icon-user"
          />
        </el-form-item>

        <el-form-item prop="email">
          <el-input
            v-model="registerForm.email"
            placeholder="邮箱"
            name="email"
            type="email"
            autocomplete="on"
            prefix-icon="el-icon-message"
          />
        </el-form-item>

        <el-form-item prop="password">
          <el-input
            ref="password"
            v-model="registerForm.password"
            placeholder="密码 (至少6位字符)"
            name="password"
            :type="passwordType"
            autocomplete="on"
            prefix-icon="el-icon-lock"
          >
            <i
              slot="suffix"
              class="el-icon-view password-icon"
              @click="showPassword"
            />
          </el-input>
        </el-form-item>

        <el-form-item prop="confirmPassword">
          <el-input
            v-model="registerForm.confirmPassword"
            placeholder="确认密码"
            name="confirmPassword"
            :type="passwordType"
            autocomplete="on"
            prefix-icon="el-icon-lock"
            @keyup.enter.native="handleRegister"
          />
        </el-form-item>

        <el-button :loading="loading" type="primary" style="width:100%;margin-bottom:30px;" @click.native.prevent="handleRegister">注册</el-button>

        <div class="tips">
          <span>已有账号？ </span>
          <router-link to="/login">立即登录</router-link>
        </div>
      </el-form>
    </el-card>
  </div>
</template>

<script>
import { register } from '@/api/user'

export default {
  name: 'Register',
  data() {
    const validateUsername = (rule, value, callback) => {
      if (!value) {
        callback(new Error('请输入用户名'))
      } else {
        callback()
      }
    }
    const validatePassword = (rule, value, callback) => {
      if (value.length < 6) {
        callback(new Error('密码不能少于6位字符'))
      } else {
        callback()
      }
    }
    const validateConfirmPassword = (rule, value, callback) => {
      if (value !== this.registerForm.password) {
        callback(new Error('两次输入密码不一致'))
      } else {
        callback()
      }
    }
    const validateEmail = (rule, value, callback) => {
      const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/
      if (!emailRegex.test(value)) {
        callback(new Error('请输入有效的邮箱地址'))
      } else {
        callback()
      }
    }
    return {
      registerForm: {
        username: '',
        password: '',
        confirmPassword: '',
        email: ''
      },
      registerRules: {
        username: [{ required: true, trigger: 'blur', validator: validateUsername }],
        password: [{ required: true, trigger: 'blur', validator: validatePassword }],
        confirmPassword: [{ required: true, trigger: 'blur', validator: validateConfirmPassword }],
        email: [{ required: true, trigger: 'blur', validator: validateEmail }]
      },
      passwordType: 'password',
      loading: false
    }
  },
  methods: {
    showPassword() {
      this.passwordType = this.passwordType === 'password' ? '' : 'password'
      this.$nextTick(() => {
        this.$refs.password.focus()
      })
    },
    handleRegister() {
      this.$refs.registerForm.validate(valid => {
        if (valid) {
          this.loading = true
          const { username, password, email } = this.registerForm
          register({ username, password, email })
            .then(response => {
              this.$message.success('注册成功，请登录')
              this.$router.push('/login')
            })
            .catch(error => {
              console.error('注册失败:', error)
              this.$message.error(error.response?.data?.message || '注册失败，请稍后重试')
            })
            .finally(() => {
              this.loading = false
            })
        } else {
          return false
        }
      })
    }
  }
}
</script>

<style lang="scss" scoped>
.register-container {
  display: flex;
  justify-content: center;
  align-items: center;
  height: 100vh;
  background-color: #f3f3f3;
  
  .register-card {
    width: 420px;
    padding: 20px;
    
    .title-container {
      text-align: center;
      margin-bottom: 30px;
      
      .title {
        font-size: 26px;
        color: #409EFF;
        font-weight: bold;
      }
    }
    
    .tips {
      margin-top: 20px;
      text-align: right;
      
      span {
        color: #999;
      }
      
      a {
        color: #409EFF;
        text-decoration: none;
      }
    }
    
    .password-icon {
      cursor: pointer;
    }
  }
}
</style> 