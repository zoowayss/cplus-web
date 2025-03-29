<!-- 主页面组件 - 作为系统的门户 -->
<template>
  <div class="main-page">
    <header class="header">
      <div class="container">
        <div class="logo">
          <h1>在线评测系统</h1>
        </div>
        <div class="nav">
          <el-button type="text" @click="goToPage('/login')">登录</el-button>
          <el-button type="primary" @click="goToPage('/register')">注册</el-button>
        </div>
      </div>
    </header>

    <section class="hero">
      <div class="container">
        <div class="hero-content">
          <h1>提升你的编程能力</h1>
          <h2>通过实践解决各种算法和编程挑战</h2>
          <el-button type="primary" size="large" @click="goToPage('/login')">开始使用</el-button>
        </div>
        <div class="hero-image">
          <img src="@/assets/hero-image.svg" alt="编程挑战" />
        </div>
      </div>
    </section>

    <section class="features">
      <div class="container">
        <h2 class="section-title">系统特点</h2>
        <div class="feature-grid">
          <div class="feature-item">
            <i class="el-icon-edit-outline"></i>
            <h3>丰富的题库</h3>
            <p>涵盖从基础到高级的各类编程题目，帮助你全面提升编程能力</p>
          </div>
          <div class="feature-item">
            <i class="el-icon-data-analysis"></i>
            <h3>实时评测</h3>
            <p>提交代码后立即获得评测结果，快速了解代码性能</p>
          </div>
          <div class="feature-item">
            <i class="el-icon-trophy"></i>
            <h3>排行榜系统</h3>
            <p>与其他用户比拼，查看解题进度和效率排名</p>
          </div>
          <div class="feature-item">
            <i class="el-icon-s-claim"></i>
            <h3>详细题解</h3>
            <p>查看官方题解和其他用户的解题思路，拓展解题思维</p>
          </div>
        </div>
      </div>
    </section>

    <section class="call-to-action">
      <div class="container">
        <h2>准备好迎接挑战了吗？</h2>
        <p>加入我们，开始你的编程之旅</p>
        <div class="action-buttons">
          <el-button type="primary" @click="goToPage('/register')">立即注册</el-button>
          <el-button @click="goToPage('/login')">登录账号</el-button>
        </div>
      </div>
    </section>

    <footer class="footer">
      <div class="container">
        <p>&copy; 2023 在线评测系统 - 版权所有</p>
      </div>
    </footer>
  </div>
</template>

<script>
export default {
  name: 'MainPage',
  data() {
    return {}
  },
  created() {
    // 如果用户已登录，根据角色重定向到相应页面
    if (this.$store.getters.isAuthenticated) {
      const userRole = this.$store.state.user ? this.$store.state.user.role : null
      const isAdmin = userRole >= 2 // 角色值>=2为管理员
      this.$router.push(isAdmin ? '/admin' : '/user')
    }
  },
  methods: {
    // 统一的导航方法
    goToPage(path) {
      if (this.$store.getters.isAuthenticated) {
        const userRole = this.$store.state.user ? this.$store.state.user.role : null
        const isAdmin = userRole >= 2
        
        // 如果已登录，根据角色导航到相应主页
        this.$router.push(isAdmin ? '/admin' : '/user')
      } else {
        // 未登录则正常导航
        this.$router.push(path)
      }
    }
  }
}
</script>

<style lang="scss" scoped>
.main-page {
  font-family: 'Helvetica Neue', Helvetica, 'PingFang SC', 'Hiragino Sans GB', 'Microsoft YaHei', Arial, sans-serif;
  color: #333;
  line-height: 1.5;

  .container {
    max-width: 1200px;
    margin: 0 auto;
    padding: 0 20px;
  }

  .header {
    background-color: #fff;
    box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
    padding: 15px 0;

    .container {
      display: flex;
      justify-content: space-between;
      align-items: center;
    }

    .logo h1 {
      margin: 0;
      font-size: 24px;
      color: #409EFF;
    }

    .nav {
      display: flex;
      gap: 15px;
    }
  }

  .hero {
    background: linear-gradient(135deg, #409EFF 0%, #53a8ff 100%);
    color: white;
    padding: 80px 0;

    .container {
      display: flex;
      align-items: center;
      gap: 40px;

      @media (max-width: 768px) {
        flex-direction: column;
        text-align: center;
      }
    }

    .hero-content {
      flex: 1;

      h1 {
        font-size: 48px;
        margin: 0 0 20px;
        font-weight: 600;

        @media (max-width: 768px) {
          font-size: 36px;
        }
      }

      h2 {
        font-size: 24px;
        font-weight: 400;
        margin: 0 0 30px;
        opacity: 0.9;

        @media (max-width: 768px) {
          font-size: 20px;
        }
      }
    }

    .hero-image {
      flex: 1;
      max-width: 500px;

      img {
        width: 100%;
        height: auto;
      }

      @media (max-width: 768px) {
        max-width: 350px;
      }
    }
  }

  .features {
    padding: 80px 0;
    background-color: #f8f9fa;

    .section-title {
      text-align: center;
      font-size: 36px;
      margin-bottom: 60px;
      position: relative;
      
      &:after {
        content: '';
        display: block;
        width: 60px;
        height: 4px;
        background-color: #409EFF;
        position: absolute;
        bottom: -15px;
        left: 50%;
        transform: translateX(-50%);
      }
    }

    .feature-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
      gap: 40px;
    }

    .feature-item {
      text-align: center;
      padding: 30px 20px;
      background-color: #fff;
      border-radius: 8px;
      box-shadow: 0 5px 15px rgba(0, 0, 0, 0.05);
      transition: transform 0.3s ease;

      &:hover {
        transform: translateY(-10px);
      }

      i {
        font-size: 48px;
        color: #409EFF;
        margin-bottom: 20px;
      }

      h3 {
        font-size: 20px;
        margin-bottom: 15px;
      }

      p {
        color: #666;
        line-height: 1.6;
      }
    }
  }

  .call-to-action {
    background: linear-gradient(135deg, #36d1dc 0%, #5b86e5 100%);
    color: white;
    text-align: center;
    padding: 80px 0;

    h2 {
      font-size: 36px;
      margin-bottom: 20px;
    }

    p {
      font-size: 18px;
      margin-bottom: 30px;
      opacity: 0.9;
    }

    .action-buttons {
      display: flex;
      justify-content: center;
      gap: 20px;

      @media (max-width: 576px) {
        flex-direction: column;
        align-items: center;
      }
    }
  }

  .footer {
    background-color: #304156;
    color: #bfcbd9;
    text-align: center;
    padding: 30px 0;

    p {
      margin: 0;
    }
  }
}
</style> 