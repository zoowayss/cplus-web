<template>
  <div class="content-section">              
    <h3>题目描述</h3>
    <div class="markdown-content">{{ problem.description }}</div>
    
    <h3>输入格式</h3>
    <div class="markdown-content">{{ problem.input_format }}</div>
    
    <h3>输出格式</h3>
    <div class="markdown-content">{{ problem.output_format }}</div>
    
    <h3>示例</h3>
    <div class="example-section">
      <div class="example-input">
        <div class="example-title">输入:</div>
        <pre>{{ problem.example_input }}</pre>
      </div>
      <div class="example-output">
        <div class="example-title">输出:</div>
        <pre>{{ problem.example_output }}</pre>
      </div>
    </div>
    
    <div v-if="problem.hint">
      <h3>提示</h3>
      <div class="markdown-content">{{ problem.hint }}</div>
    </div>
  </div>
</template>

<script>
export default {
  name: 'ProblemDescription',
  props: {
    problem: {
      type: Object,
      required: true
    }
  }
}
</script>

<style lang="scss" scoped>
.content-section {
  margin-top: 20px;
  
  h3 {
    margin-top: 20px;
    margin-bottom: 10px;
    font-size: 18px;
    font-weight: 600;
    color: #333;
  }
  
  .markdown-content {
    line-height: 1.6;
    white-space: pre-wrap;
    margin-bottom: 15px;
  }
}

.example-section {
  display: flex;
  flex-direction: column;
  margin-bottom: 20px;
  
  .example-input,
  .example-output {
    margin-bottom: 15px;
    
    .example-title {
      font-weight: 600;
      margin-bottom: 5px;
    }
    
    pre {
      background-color: #f5f7fa;
      padding: 10px;
      border-radius: 4px;
      overflow-x: auto;
      line-height: 1.5;
      max-height: 200px;
    }
  }
}
</style> 