<template>
  <div class="problem-edit-container">
    <el-card class="problem-edit-card">
      <div slot="header">
        <span>{{ isEdit ? '编辑题目' : '创建题目' }}</span>
      </div>
      
      <el-form :model="problemForm" :rules="rules" ref="problemForm" label-width="100px" v-loading="loading">
        <!-- 基本信息 -->
        <el-form-item label="标题" prop="title">
          <el-input v-model="problemForm.title" placeholder="请输入题目标题" />
        </el-form-item>
        
        <el-form-item label="难度" prop="difficulty">
          <el-select v-model="problemForm.difficulty" placeholder="请选择难度">
            <el-option label="简单" value="简单" />
            <el-option label="中等" value="中等" />
            <el-option label="困难" value="困难" />
          </el-select>
        </el-form-item>
        
        <el-form-item label="状态" prop="status">
          <el-radio-group v-model="problemForm.status">
            <el-radio :label="1">启用</el-radio>
            <el-radio :label="0">禁用</el-radio>
          </el-radio-group>
        </el-form-item>
        
        <el-form-item label="时间限制" prop="time_limit">
          <el-input-number v-model="problemForm.time_limit" :min="100" :max="10000" :step="100" />
          <span class="input-suffix">毫秒</span>
        </el-form-item>
        
        <el-form-item label="内存限制" prop="memory_limit">
          <el-input-number v-model="problemForm.memory_limit" :min="16384" :max="262144" :step="16384" />
          <span class="input-suffix">KB</span>
        </el-form-item>
        
        <!-- 详细内容 -->
        <el-tabs v-model="activeTab" type="card">
          <el-tab-pane label="题目描述" name="description">
            <el-form-item prop="description">
              <el-input
                v-model="problemForm.description"
                type="textarea"
                :rows="10"
                placeholder="请输入题目描述，支持 Markdown 格式"
              />
            </el-form-item>
          </el-tab-pane>
          
          <el-tab-pane label="输入格式" name="input_format">
            <el-form-item prop="input_format">
              <el-input
                v-model="problemForm.input_format"
                type="textarea"
                :rows="10"
                placeholder="请输入输入格式说明"
              />
            </el-form-item>
          </el-tab-pane>
          
          <el-tab-pane label="输出格式" name="output_format">
            <el-form-item prop="output_format">
              <el-input
                v-model="problemForm.output_format"
                type="textarea"
                :rows="10"
                placeholder="请输入输出格式说明"
              />
            </el-form-item>
          </el-tab-pane>
          
          <el-tab-pane label="示例" name="example">
            <el-form-item label="示例输入" prop="example_input">
              <el-input
                v-model="problemForm.example_input"
                type="textarea"
                :rows="5"
                placeholder="请输入示例输入"
              />
            </el-form-item>
            
            <el-form-item label="示例输出" prop="example_output">
              <el-input
                v-model="problemForm.example_output"
                type="textarea"
                :rows="5"
                placeholder="请输入示例输出"
              />
            </el-form-item>
          </el-tab-pane>
          
          <el-tab-pane label="提示" name="hint">
            <el-form-item prop="hint">
              <el-input
                v-model="problemForm.hint"
                type="textarea"
                :rows="10"
                placeholder="请输入提示信息（可选）"
              />
            </el-form-item>
          </el-tab-pane>
        </el-tabs>
        
        <!-- 操作按钮 -->
        <el-form-item>
          <el-button type="primary" @click="submitForm">{{ isEdit ? '保存修改' : '创建题目' }}</el-button>
          <el-button @click="cancelEdit">取消</el-button>
        </el-form-item>
      </el-form>
    </el-card>
  </div>
</template>

<script>
import { getProblemDetail, createProblem, updateProblem } from '@/api/problem';

export default {
  name: 'ProblemEdit',
  props: {
    params: {
      type: Object,
      default: () => ({})
    }
  },
  data() {
    return {
      loading: false,
      isEdit: false,
      problemId: null,
      activeTab: 'description',
      problemForm: {
        title: '',
        description: '',
        input_format: '',
        output_format: '',
        difficulty: '中等',
        time_limit: 1000,
        memory_limit: 65536,
        example_input: '',
        example_output: '',
        hint: '',
        status: 1
      },
      rules: {
        title: [
          { required: true, message: '请输入题目标题', trigger: 'blur' },
          { min: 1, max: 255, message: '标题长度应在1-255个字符之间', trigger: 'blur' }
        ],
        description: [
          { required: true, message: '请输入题目描述', trigger: 'blur' }
        ],
        input_format: [
          { required: true, message: '请输入输入格式', trigger: 'blur' }
        ],
        output_format: [
          { required: true, message: '请输入输出格式', trigger: 'blur' }
        ],
        difficulty: [
          { required: true, message: '请选择难度', trigger: 'change' }
        ],
        status: [
          { required: true, message: '请选择状态', trigger: 'change' }
        ],
        time_limit: [
          { required: true, message: '请设置时间限制', trigger: 'blur' },
          { type: 'number', min: 100, max: 10000, message: '时间限制在100-10000毫秒之间', trigger: 'blur' }
        ],
        memory_limit: [
          { required: true, message: '请设置内存限制', trigger: 'blur' },
          { type: 'number', min: 16384, max: 262144, message: '内存限制在16MB-256MB之间', trigger: 'blur' }
        ]
      }
    };
  },
  created() {
    // 优先使用 props 传递的参数
    if (this.params && this.params.id) {
      this.isEdit = true;
      this.problemId = parseInt(this.params.id);
      this.fetchProblemDetail();
      return;
    }
    
    // 回退到从路由参数获取ID
    const { id } = this.$route.params;
    if (id) {
      this.isEdit = true;
      this.problemId = parseInt(id);
      this.fetchProblemDetail();
    }
  },
  methods: {
    // 获取题目详情
    fetchProblemDetail() {
      if (!this.problemId) return;
      
      this.loading = true;
      getProblemDetail(this.problemId)
        .then(response => {
          if (response.status === 'ok' && response.problem) {
            // 填充表单数据
            const problem = response.problem;
            this.problemForm = {
              title: problem.title || '',
              description: problem.description || '',
              input_format: problem.input_format || '',
              output_format: problem.output_format || '',
              difficulty: problem.difficulty || '中等',
              time_limit: problem.time_limit || 1000,
              memory_limit: problem.memory_limit || 65536,
              example_input: problem.example_input || '',
              example_output: problem.example_output || '',
              hint: problem.hint || '',
              status: problem.status !== undefined ? problem.status : 1
            };
          } else {
            this.$message.error(response.message || '获取题目详情失败');
            this.$router.push('/admin/problems');
          }
        })
        .catch(error => {
          console.error('获取题目详情失败:', error);
          this.$message.error('获取题目详情失败，请稍后再试');
          this.$router.push('/admin/problems');
        })
        .finally(() => {
          this.loading = false;
        });
    },
    
    // 提交表单
    submitForm() {
      this.$refs.problemForm.validate(valid => {
        if (valid) {
          this.loading = true;
          
          const problemData = { ...this.problemForm };
          
          // 根据是否编辑模式选择不同的API
          const apiCall = this.isEdit
            ? updateProblem(this.problemId, problemData)
            : createProblem(problemData);
          
          apiCall
            .then(response => {
              if (response.status === 'ok') {
                this.$message.success(this.isEdit ? '更新题目成功' : '创建题目成功');
                
                // 如果是创建模式，成功后转到编辑页面
                if (!this.isEdit && response.problem_id) {
                  this.$router.push(`/admin/problems/edit/${response.problem_id}`);
                } else {
                  // 编辑模式成功后返回列表
                  this.$router.push('/admin/problems');
                }
              } else {
                this.$message.error(response.message || (this.isEdit ? '更新题目失败' : '创建题目失败'));
              }
            })
            .catch(error => {
              console.error(this.isEdit ? '更新题目失败:' : '创建题目失败:', error);
              this.$message.error(this.isEdit ? '更新题目失败，请稍后再试' : '创建题目失败，请稍后再试');
            })
            .finally(() => {
              this.loading = false;
            });
        } else {
          this.$message.warning('请正确填写表单');
          return false;
        }
      });
    },
    
    // 取消编辑
    cancelEdit() {
      this.$router.push('/admin/problems');
    }
  }
};
</script>

<style lang="scss" scoped>
.problem-edit-container {
  padding: 20px;
  
  .problem-edit-card {
    margin-bottom: 20px;
    
    ::v-deep .el-tabs__header {
      margin-bottom: 20px;
    }
  }
  
  .input-suffix {
    margin-left: 10px;
    color: #606266;
  }
}
</style> 