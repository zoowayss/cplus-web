<template>
  <div class="submit-section">
    <h3>代码编辑器</h3>
    
    <div class="language-selector">
      <span>编程语言：</span>
      <el-tag type="primary">C++</el-tag>
    </div>
    
    <!-- 代码编辑器 -->
    <div class="code-editor-wrapper">
      <el-input
        type="textarea"
        v-model="localCode"
        :rows="20"
        placeholder="在此输入您的C++代码"
        class="code-editor"
      />
    </div>
    
    <!-- 提交按钮 -->
    <div class="submit-button-container">
      <el-button type="primary" @click="handleSubmit" :loading="submitting">提交代码</el-button>
    </div>
  </div>
</template>

<script>
export default {
  name: 'CodeSubmission',
  props: {
    code: {
      type: String,
      default: ''
    },
    submitting: {
      type: Boolean,
      default: false
    }
  },
  data() {
    return {
      localCode: this.code,
      localLanguage: 'cpp' // 固定为cpp
    }
  },
  watch: {
    code(val) {
      this.localCode = val;
    },
    localCode(val) {
      this.$emit('update:code', val);
    }
  },
  methods: {
    handleSubmit() {
      // 发送固定的cpp语言
      this.$emit('update:language', 'cpp');
      this.$emit('submit');
    }
  }
}
</script>

<style lang="scss" scoped>
.submit-section {
  padding: 15px 0;
  
  h3 {
    margin-top: 0;
    margin-bottom: 15px;
  }
  
  .language-selector {
    margin-bottom: 15px;
    display: flex;
    align-items: center;
    
    span {
      margin-right: 10px;
    }
  }
  
  .code-editor-wrapper {
    margin-bottom: 15px;
    
    .code-editor {
      font-family: 'Courier New', monospace;
    }
  }
  
  .submit-button-container {
    text-align: right;
  }
}
</style> 