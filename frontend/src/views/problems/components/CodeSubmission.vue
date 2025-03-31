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
    
    <!-- 提交按钮和重置按钮 -->
    <div class="submit-button-container">
      <el-button type="default" @click="resetToTemplate" :disabled="!templateAvailable">重置为模板</el-button>
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
    },
    problem: {
      type: Object,
      default: () => ({})
    }
  },
  data() {
    return {
      localCode: this.code,
      localLanguage: 'cpp', // 固定为cpp
      originalTemplate: '', // 存储原始模板
      templateApplied: false // 标记是否已应用过模板
    }
  },
  computed: {
    templateAvailable() {
      return this.originalTemplate && this.originalTemplate.length > 0;
    }
  },
  watch: {
    code(val) {
      // 只有在代码发生变化且不是初始加载时更新本地代码
      if (val !== this.localCode) {
        this.localCode = val;
      }
    },
    localCode(val) {
      this.$emit('update:code', val);
    },
    'problem.code_template': {
      handler(newTemplate) {
        if (newTemplate && !this.templateApplied) {
          // 如果存在模板并且尚未应用过，则应用模板
          this.originalTemplate = newTemplate;
          if (!this.localCode || this.localCode.trim() === '') {
            this.localCode = newTemplate;
            this.templateApplied = true;
          }
        }
      },
      immediate: true // 在组件创建时立即执行一次
    },
    problem: {
      handler(newProblem) {
        if (newProblem && newProblem.code_template && !this.templateApplied) {
          this.originalTemplate = newProblem.code_template;
          if (!this.localCode || this.localCode.trim() === '') {
            this.localCode = newProblem.code_template;
            this.templateApplied = true;
          }
        }
      },
      immediate: true,
      deep: true
    }
  },
  methods: {
    handleSubmit() {
      // 发送固定的cpp语言
      this.$emit('update:language', 'cpp');
      this.$emit('submit');
    },
    resetToTemplate() {
      if (this.originalTemplate) {
        this.$confirm('确定要重置为原始模板吗？这将丢失您当前的所有代码。', '警告', {
          confirmButtonText: '确定',
          cancelButtonText: '取消',
          type: 'warning'
        }).then(() => {
          this.localCode = this.originalTemplate;
          this.$message.success('已重置为原始模板');
        }).catch(() => {
          // 用户取消操作
        });
      }
    }
  },
  mounted() {
    // 在组件挂载后检查是否需要应用模板
    if (this.problem && this.problem.code_template && !this.templateApplied) {
      this.originalTemplate = this.problem.code_template;
      if (!this.localCode || this.localCode.trim() === '') {
        this.localCode = this.problem.code_template;
        this.templateApplied = true;
      }
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
    
    button {
      margin-left: 10px;
    }
  }
}
</style> 