<template>
  <div class="testcase-manager-container">
    <el-card class="testcase-manager-card">
      <div slot="header" class="card-header">
        <span>测试用例管理 - {{ problem.title }}</span>
        <div class="card-header-buttons">
          <el-button size="small" @click="editProblem()">
            <i class="el-icon-edit"></i> 编辑题目
          </el-button>
          <el-button size="small" @click="goBackToList()">
            <i class="el-icon-back"></i> 返回题目列表
          </el-button>
        </div>
      </div>
      
      <div v-loading="loading">
        <!-- 添加测试用例按钮 -->
        <div class="button-container">
          <el-button type="primary" @click="showAddDialog">
            <i class="el-icon-plus"></i> 添加测试用例
          </el-button>
        </div>
        
        <!-- 测试用例列表 -->
        <div v-if="testcases && testcases.length > 0">
          <div 
            v-for="(testcase, index) in testcases" 
            :key="index" 
            class="testcase-item"
            :class="{ 'is-example': testcase.is_example }">
            <div class="testcase-header">
              <div class="testcase-title">
                <span>测试用例 #{{ index + 1 }}</span>
                <el-tag v-if="testcase.is_example" size="small" type="success">示例</el-tag>
              </div>
              <div class="testcase-actions">
                <el-button type="text" size="small" @click="editTestCase(testcase)">
                  <i class="el-icon-edit"></i> 编辑
                </el-button>
                <el-button type="text" size="small" class="danger-text" @click="confirmDelete(testcase)">
                  <i class="el-icon-delete"></i> 删除
                </el-button>
              </div>
            </div>
            
            <div class="testcase-content">
              <div class="content-row">
                <div class="content-label">输入:</div>
                <div class="content-value">
                  <pre>{{ testcase.input || '(空)' }}</pre>
                </div>
              </div>
              <div class="content-row">
                <div class="content-label">期望输出:</div>
                <div class="content-value">
                  <pre>{{ testcase.expected_output || '(空)' }}</pre>
                </div>
              </div>
              <div class="content-info">
                <span>创建时间: {{ formatDate(testcase.created_at) }}</span>
                <span class="testcase-id">ID: {{ testcase.id }}</span>
              </div>
            </div>
          </div>
        </div>
        
        <!-- 空状态 -->
        <el-empty v-else description="暂无测试用例，请添加">
          <el-button type="primary" @click="showAddDialog">添加测试用例</el-button>
        </el-empty>
      </div>
    </el-card>
    
    <!-- 添加/编辑测试用例对话框 -->
    <el-dialog
      :title="isEdit ? '编辑测试用例' : '添加测试用例'"
      :visible.sync="dialogVisible"
      width="50%"
      :before-close="handleDialogClose">
      <el-form :model="testcaseForm" :rules="rules" ref="testcaseForm" label-width="100px">
        <el-form-item label="输入" prop="input">
          <el-input
            type="textarea"
            v-model="testcaseForm.input"
            :rows="10"
            placeholder="请输入测试用例的输入数据"
          />
        </el-form-item>
        
        <el-form-item label="期望输出" prop="expected_output">
          <el-input
            type="textarea"
            v-model="testcaseForm.expected_output"
            :rows="10"
            placeholder="请输入测试用例的期望输出"
          />
        </el-form-item>
        
        <el-form-item label="是否示例">
          <el-switch v-model="testcaseForm.is_example" />
          <span class="help-text">设置为示例将显示在题目详情页面</span>
        </el-form-item>
      </el-form>
      
      <span slot="footer" class="dialog-footer">
        <el-button @click="dialogVisible = false">取消</el-button>
        <el-button type="primary" @click="submitTestCase" :loading="submitting">确定</el-button>
      </span>
    </el-dialog>
    
    <!-- 删除确认对话框 -->
    <el-dialog
      title="确认删除"
      :visible.sync="deleteDialogVisible"
      width="30%">
      <span>确定要删除这个测试用例吗？此操作不可恢复。</span>
      <span slot="footer" class="dialog-footer">
        <el-button @click="deleteDialogVisible = false">取消</el-button>
        <el-button type="danger" @click="deleteTestCase" :loading="submitting">确定删除</el-button>
      </span>
    </el-dialog>
  </div>
</template>

<script>
import { getProblemDetail, addTestCase, updateTestCase, deleteTestCase } from '@/api/problem';

export default {
  name: 'TestCaseManager',
  props: {
    params: {
      type: Object,
      default: () => ({})
    }
  },
  data() {
    return {
      loading: false,
      submitting: false,
      problem: {},
      problemId: null,
      testcases: [],
      dialogVisible: false,
      isEdit: false,
      testcaseForm: {
        id: null,
        input: '',
        expected_output: '',
        is_example: false
      },
      rules: {
        input: [
          { required: true, message: '请输入测试用例输入', trigger: 'blur' }
        ],
        expected_output: [
          { required: true, message: '请输入测试用例期望输出', trigger: 'blur' }
        ]
      },
      deleteDialogVisible: false,
      testcaseToDelete: null
    };
  },
  created() {
    console.log('TestCaseManager组件创建，params:', this.params);
    
    // 优先使用 props 传递的参数
    if (this.params && this.params.id) {
      this.problemId = parseInt(this.params.id);
      console.log('使用props获取problemId:', this.problemId);
      this.initTestCaseManager();
      return;
    }
    
    // 获取题目ID从路由参数
    const { id } = this.$route.params;
    if (id) {
      this.problemId = parseInt(id);
      console.log('使用route.params获取problemId:', this.problemId);
      this.initTestCaseManager();
    } else {
      console.error('无法获取题目ID');
      this.$message.error('无效的题目ID');
      this.$router.push('/admin/problems');
    }
  },
  methods: {
    // 初始化测试用例管理器
    initTestCaseManager() {
      if (!this.problemId || isNaN(this.problemId)) {
        console.error('无效的problemId:', this.problemId);
        this.$message.error('无效的题目ID');
        return;
      }
      
      // 只调用获取题目详情API，因为它也会包含测试用例数据
      this.fetchProblemDetail();
    },
    
    // 获取题目详情
    fetchProblemDetail() {
      this.loading = true;
      return getProblemDetail(this.problemId)
        .then(response => {
          console.log('获取题目详情响应:', response);
          if (response.status === 'ok' && response.problem) {
            this.problem = response.problem;
            console.log('题目详情获取成功:', this.problem.title);
            
            // 提取测试用例数据
            if (this.problem.testcases && Array.isArray(this.problem.testcases)) {
              console.log('获取到测试用例数据:', this.problem.testcases.length);
              this.testcases = this.problem.testcases;
              if (this.testcases.length > 0) {
                this.$message.success(`成功加载 ${this.testcases.length} 个测试用例`);
              } else {
                this.$message.info('该题目暂无测试用例，请添加');
              }
            } else {
              console.warn('题目中没有包含测试用例数据');
              this.testcases = [];
              this.$message.info('该题目暂无测试用例，请添加');
            }
          } else {
            console.error('获取题目详情失败:', response.message);
            this.$message.error(response.message || '获取题目详情失败');
            this.$router.push('/admin/problems');
          }
        })
        .catch(error => {
          console.error('获取题目详情异常:', error);
          this.$message.error('获取题目详情失败，请稍后再试');
          this.$router.push('/admin/problems');
        })
        .finally(() => {
          this.loading = false;
        });
    },
    
    // 刷新数据
    refreshData() {
      // 延迟一下再重新加载数据，确保服务器已处理完成
      setTimeout(() => {
        this.fetchProblemDetail();
      }, 500);
    },
    
    // 显示添加对话框
    showAddDialog() {
      this.isEdit = false;
      this.testcaseForm = {
        id: null,
        input: '',
        expected_output: '',
        is_example: false
      };
      this.dialogVisible = true;
      
      // 下一轮事件循环中重置表单校验
      this.$nextTick(() => {
        this.$refs.testcaseForm && this.$refs.testcaseForm.clearValidate();
      });
    },
    
    // 编辑测试用例
    editTestCase(testcase) {
      this.isEdit = true;
      this.testcaseForm = {
        id: testcase.id,
        input: testcase.input,
        expected_output: testcase.expected_output,
        is_example: testcase.is_example
      };
      this.dialogVisible = true;
      
      // 下一轮事件循环中重置表单校验
      this.$nextTick(() => {
        this.$refs.testcaseForm && this.$refs.testcaseForm.clearValidate();
      });
    },
    
    // 提交测试用例（添加/编辑）
    submitTestCase() {
      this.$refs.testcaseForm.validate(valid => {
        if (valid) {
          this.submitting = true;
          
          console.log('提交测试用例数据:', this.testcaseForm);
          
          // 确保表单数据是期望的格式
          const formData = {
            ...this.testcaseForm,
            problem_id: this.problemId  // 确保包含problem_id
          };
          
          const apiCall = this.isEdit
            ? updateTestCase(this.testcaseForm.id, formData)
            : addTestCase(this.problemId, formData);
          
          apiCall
            .then(response => {
              console.log('提交测试用例响应:', response);
              if (response.status === 'ok') {
                this.$message.success(this.isEdit ? '更新测试用例成功' : '添加测试用例成功');
                
                // 关闭对话框
                this.dialogVisible = false;
                
                // 刷新数据
                this.refreshData();
              } else {
                this.$message.error(response.message || (this.isEdit ? '更新测试用例失败' : '添加测试用例失败'));
              }
            })
            .catch(error => {
              console.error(this.isEdit ? '更新测试用例失败:' : '添加测试用例失败:', error);
              this.$message.error(this.isEdit ? '更新测试用例失败，请稍后再试' : '添加测试用例失败，请稍后再试');
            })
            .finally(() => {
              this.submitting = false;
            });
        } else {
          this.$message.warning('请正确填写表单');
          return false;
        }
      });
    },
    
    // 确认删除对话框
    confirmDelete(testcase) {
      this.testcaseToDelete = testcase;
      this.deleteDialogVisible = true;
    },
    
    // 删除测试用例
    deleteTestCase() {
      if (!this.testcaseToDelete) return;
      
      this.submitting = true;
      deleteTestCase(this.testcaseToDelete.id)
        .then(response => {
          if (response.status === 'ok') {
            this.$message.success('删除测试用例成功');
            // 关闭对话框
            this.deleteDialogVisible = false;
            
            // 刷新数据
            this.refreshData();
          } else {
            this.$message.error(response.message || '删除测试用例失败');
          }
        })
        .catch(error => {
          console.error('删除测试用例失败:', error);
          this.$message.error('删除测试用例失败，请稍后再试');
        })
        .finally(() => {
          this.submitting = false;
        });
    },
    
    // 处理对话框关闭
    handleDialogClose(done) {
      if (this.submitting) {
        this.$message.warning('正在提交，请稍等...');
        return;
      }
      done();
    },
    
    // 格式化日期
    formatDate(timestamp) {
      if (!timestamp) return '';
      
      // 判断时间戳是秒级还是毫秒级
      const milliseconds = timestamp.toString().length > 10 ? timestamp : timestamp * 1000;
      
      try {
        const date = new Date(milliseconds);
        if (isNaN(date.getTime())) {
          console.error('无效的时间戳:', timestamp);
          return '日期格式无效';
        }
        return date.toLocaleString();
      } catch (error) {
        console.error('日期格式化错误:', error);
        return '日期格式错误';
      }
    },
    
    // 编辑题目
    editProblem() {
      // 直接导航到管理员题目编辑页面，添加fromAdmin参数以绕过可能的导航守卫拦截
      this.$router.push({
        path: `/admin/problems/edit/${this.problemId}`,
        query: { fromAdmin: true }
      }).catch(err => {
        // 忽略导航重复错误
        if (err.name !== 'NavigationDuplicated') {
          console.error('导航到题目编辑页面失败:', err);
          this.$message.error('导航到题目编辑页面失败');
        }
      });
    },
    
    // 返回题目列表
    goBackToList() {
      this.$router.push({
        path: '/admin/problems',
        query: { fromAdmin: true }
      }).catch(err => {
        // 忽略导航重复错误
        if (err.name !== 'NavigationDuplicated') {
          console.error('导航到题目列表失败:', err);
          this.$message.error('导航到题目列表失败');
        }
      });
    }
  }
};
</script>

<style lang="scss" scoped>
.testcase-manager-container {
  padding: 20px;
  
  .testcase-manager-card {
    .card-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      
      .card-header-buttons {
        display: flex;
        gap: 10px;
      }
    }
    
    .button-container {
      margin-bottom: 20px;
    }
    
    .testcase-item {
      border: 1px solid #ebeef5;
      border-radius: 4px;
      margin-bottom: 20px;
      transition: all 0.3s;
      
      &:hover {
        box-shadow: 0 2px 12px 0 rgba(0,0,0,0.1);
      }
      
      &.is-example {
        border-left: 4px solid #67c23a;
      }
      
      .testcase-header {
        padding: 12px 20px;
        background-color: #f5f7fa;
        border-bottom: 1px solid #ebeef5;
        display: flex;
        justify-content: space-between;
        align-items: center;
        
        .testcase-title {
          font-weight: bold;
          display: flex;
          align-items: center;
          gap: 10px;
        }
        
        .testcase-actions {
          display: flex;
          gap: 10px;
        }
      }
      
      .testcase-content {
        padding: 20px;
        
        .content-row {
          margin-bottom: 15px;
          
          .content-label {
            font-weight: bold;
            margin-bottom: 5px;
          }
          
          .content-value {
            background-color: #f5f7fa;
            border: 1px solid #ebeef5;
            border-radius: 4px;
            padding: 10px;
            overflow-x: auto;
            
            pre {
              margin: 0;
              white-space: pre-wrap;
              word-break: break-all;
            }
          }
        }
        
        .content-info {
          font-size: 12px;
          color: #909399;
          display: flex;
          justify-content: space-between;
          
          .testcase-id {
            color: #909399;
          }
        }
      }
    }
  }
  
  .danger-text {
    color: #f56c6c;
  }
  
  .help-text {
    margin-left: 10px;
    color: #909399;
    font-size: 12px;
  }
}
</style> 