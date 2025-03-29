<template>
  <div class="submission-list-container">
    <el-card class="submission-list-card">
      <div slot="header" class="card-header">
        <span>我的提交记录</span>
        <div class="card-header-buttons">
          <el-button size="small" icon="el-icon-refresh" @click="fetchSubmissions">刷新</el-button>
        </div>
      </div>
      
      <!-- 筛选条件 -->
      <div class="filter-bar">
        <el-form :inline="true" :model="filterForm" class="form-inline">
          <el-form-item label="题目ID">
            <el-input v-model="filterForm.problemId" placeholder="输入题目ID" clearable></el-input>
          </el-form-item>
          <el-form-item label="状态">
            <el-select v-model="filterForm.status" placeholder="选择状态" clearable>
              <el-option v-for="item in statusOptions" :key="item.value" :label="item.label" :value="item.value"></el-option>
            </el-select>
          </el-form-item>
          <el-form-item label="语言">
            <el-select v-model="filterForm.language" placeholder="选择语言" clearable>
              <el-option v-for="item in languageOptions" :key="item.value" :label="item.label" :value="item.value"></el-option>
            </el-select>
          </el-form-item>
          <el-form-item>
            <el-button type="primary" @click="handleFilter">查询</el-button>
            <el-button @click="resetFilter">重置</el-button>
          </el-form-item>
        </el-form>
      </div>
      
      <!-- 提交记录表格 -->
      <el-table
        :data="submissions"
        border
        style="width: 100%"
        v-loading="loading"
      >
        <el-table-column prop="id" label="ID" width="80" />
        <el-table-column prop="problem_id" label="题目ID" width="80" />
        <el-table-column prop="problem_title" label="题目标题" min-width="180" />
        <el-table-column prop="language" label="语言" width="100" />
        <el-table-column label="状态" width="120">
          <template slot-scope="scope">
            <el-tag :type="getSubmissionStatusType(scope.row.status)" size="small">
              {{ scope.row.status }}
            </el-tag>
          </template>
        </el-table-column>
        <el-table-column label="运行时间" width="110">
          <template slot-scope="scope">
            <span>{{ scope.row.runtime }} ms</span>
          </template>
        </el-table-column>
        <el-table-column label="内存占用" width="110">
          <template slot-scope="scope">
            <span>{{ scope.row.memory }} KB</span>
          </template>
        </el-table-column>
        <el-table-column label="提交时间" width="180">
          <template slot-scope="scope">
            <span>{{ formatDate(scope.row.created_at) }}</span>
          </template>
        </el-table-column>
        <el-table-column label="操作" width="160">
          <template slot-scope="scope">
            <el-button type="text" size="small" @click="viewSubmission(scope.row)">查看详情</el-button>
            <el-button 
              type="text" 
              size="small" 
              @click="resubmit(scope.row)"
              :disabled="scope.row.status === '评测中'"
            >重新提交</el-button>
          </template>
        </el-table-column>
      </el-table>
      
      <!-- 分页 -->
      <div class="pagination-container">
        <el-pagination
          background
          @size-change="handleSizeChange"
          @current-change="handleCurrentChange"
          :current-page="currentPage"
          :page-sizes="[10, 20, 50, 100]"
          :page-size="pageSize"
          layout="total, sizes, prev, pager, next, jumper"
          :total="total"
        />
      </div>
      
      <!-- 提交详情对话框 -->
      <el-dialog
        title="提交详情"
        :visible.sync="submissionDialogVisible"
        width="60%"
        class="submission-dialog"
      >
        <div v-if="currentSubmission" class="submission-detail">
          <div class="detail-row">
            <span class="detail-label">ID：</span>
            <span>{{ currentSubmission.id }}</span>
          </div>
          <div class="detail-row">
            <span class="detail-label">题目：</span>
            <span>{{ currentSubmission.problem_title }}</span>
          </div>
          <div class="detail-row">
            <span class="detail-label">状态：</span>
            <el-tag :type="getSubmissionStatusType(currentSubmission.status)" size="small">
              {{ currentSubmission.status }}
            </el-tag>
          </div>
          <div class="detail-row">
            <span class="detail-label">语言：</span>
            <span>{{ currentSubmission.language }}</span>
          </div>
          <div class="detail-row">
            <span class="detail-label">运行时间：</span>
            <span>{{ currentSubmission.runtime }} ms</span>
          </div>
          <div class="detail-row">
            <span class="detail-label">内存占用：</span>
            <span>{{ currentSubmission.memory }} KB</span>
          </div>
          <div class="detail-row">
            <span class="detail-label">提交时间：</span>
            <span>{{ formatDate(currentSubmission.created_at) }}</span>
          </div>
          
          <div class="code-section">
            <div class="code-header">
              <span class="code-title">提交的代码</span>
              <el-button size="mini" type="primary" icon="el-icon-copy-document" @click="copyCode">复制代码</el-button>
            </div>
            <pre class="code-content">{{ currentSubmission.code }}</pre>
          </div>
          
          <div v-if="currentSubmission.error_message" class="error-section">
            <div class="error-header">错误信息</div>
            <pre class="error-content">{{ currentSubmission.error_message }}</pre>
          </div>
        </div>
        <span slot="footer" class="dialog-footer">
          <el-button @click="submissionDialogVisible = false">关闭</el-button>
          <el-button 
            type="primary" 
            @click="resubmit(currentSubmission)" 
            :disabled="currentSubmission && currentSubmission.status === '评测中'"
          >重新提交</el-button>
        </span>
      </el-dialog>
    </el-card>
  </div>
</template>

<script>
// 导入所需的 API 函数（这些函数需要在 API 文件中定义）
// import { getUserSubmissions, getSubmissionDetail, resubmitCode } from '@/api/submission';

export default {
  name: 'SubmissionList',
  data() {
    return {
      loading: false,
      submissions: [],
      total: 0,
      currentPage: 1,
      pageSize: 10,
      filterForm: {
        problemId: '',
        status: '',
        language: ''
      },
      statusOptions: [
        { label: '通过', value: '通过' },
        { label: '编译错误', value: '编译错误' },
        { label: '运行错误', value: '运行错误' },
        { label: '超时', value: '超时' },
        { label: '评测中', value: '评测中' }
      ],
      languageOptions: [
        { label: 'C++', value: 'cpp' },
        { label: 'Java', value: 'java' },
        { label: 'Python', value: 'python' }
      ],
      submissionDialogVisible: false,
      currentSubmission: null,
      
      // 模拟数据 - 实际应从 API 获取
      mockSubmissions: [
        {
          id: 1001,
          problem_id: 1,
          problem_title: '两数之和',
          language: 'C++',
          status: '通过',
          runtime: 5,
          memory: 2048,
          created_at: Date.now() / 1000 - 3600 * 3,
          code: `#include <iostream>\n#include <vector>\n#include <unordered_map>\nusing namespace std;\n\nclass Solution {\npublic:\n    vector<int> twoSum(vector<int>& nums, int target) {\n        unordered_map<int, int> map;\n        for (int i = 0; i < nums.size(); i++) {\n            int complement = target - nums[i];\n            if (map.count(complement)) {\n                return {map[complement], i};\n            }\n            map[nums[i]] = i;\n        }\n        return {};\n    }\n};`,
          error_message: null
        },
        {
          id: 1000,
          problem_id: 2,
          problem_title: '回文数',
          language: 'Java',
          status: '编译错误',
          runtime: 0,
          memory: 0,
          created_at: Date.now() / 1000 - 3600 * 5,
          code: `public class Solution {\n    public boolean isPalindrome(int x) {\n        if (x < 0) return false;\n        \n        // 将整数转换为字符串\n        String str = Integer.toString(x);\n        int left = 0;\n        int right = str.length() - 1;\n        \n        // 从两端开始比较\n        while (left < right) {\n            if (str.charAt(left) != str.charAt(right)) {\n                return false;\n            }\n            left++;\n            right--;\n        }\n        \n        return true;\n    )\n}`,
          error_message: "Compilation error: \nSolution.java:18: error: 缺少 '}'\n        return true;\n                   )\n       1 error"
        },
        {
          id: 999,
          problem_id: 3,
          problem_title: '最长回文子串',
          language: 'Python',
          status: '运行错误',
          runtime: 2,
          memory: 1024,
          created_at: Date.now() / 1000 - 3600 * 24,
          code: `class Solution:\n    def longestPalindrome(self, s: str) -> str:\n        if not s:\n            return \"\"\n            \n        n = len(s)\n        # 创建 DP 表\n        dp = [[False for _ in range(n)] for _ in range(n)]\n        start = 0\n        max_length = 1\n        \n        # 所有长度为 1 的子串都是回文\n        for i in range(n):\n            dp[i][i] = True\n            \n        # 检查长度为 2 的子串\n        for i in range(n-1):\n            if s[i] == s[i+1]:\n                dp[i][i+1] = True\n                start = i\n                max_length = 2\n                \n        # 检查长度大于 2 的子串\n        for length in range(3, n+1):\n            for i in range(n - length + 1):\n                j = i + length - 1  # 子串的结束位置\n                if dp[i+1][j-1] and s[i] == s[j]:\n                    dp[i][j] = True\n                    if length > max_length:\n                        max_length = length\n                        start = i\n        \n        return s[start:start + max_length]`,
          error_message: "Runtime error: \nIndexError: list index out of range\n  File \"Solution.py\", line 26, in longestPalindrome\n    if dp[i+1][j-1] and s[i] == s[j]:"
        }
      ]
    };
  },
  created() {
    this.fetchSubmissions();
  },
  methods: {
    // 获取提交记录列表
    fetchSubmissions() {
      this.loading = true;
      
      // 实际项目中，应该调用 API 获取提交记录
      // 这里使用模拟数据
      setTimeout(() => {
        this.submissions = [...this.mockSubmissions];
        this.total = this.submissions.length;
        this.loading = false;
      }, 500);
      
      // 实际 API 调用方式：
      /*
      const params = {
        offset: (this.currentPage - 1) * this.pageSize,
        limit: this.pageSize,
        problem_id: this.filterForm.problemId || undefined,
        status: this.filterForm.status || undefined,
        language: this.filterForm.language || undefined
      };
      
      getUserSubmissions(params)
        .then(response => {
          if (response.status === 'ok') {
            this.submissions = response.submissions || [];
            this.total = response.total || 0;
          } else {
            this.$message.error(response.message || '获取提交记录失败');
          }
        })
        .catch(error => {
          console.error('获取提交记录失败:', error);
          this.$message.error('获取提交记录失败，请稍后再试');
        })
        .finally(() => {
          this.loading = false;
        });
      */
    },
    
    // 查看提交详情
    viewSubmission(submission) {
      this.currentSubmission = submission;
      this.submissionDialogVisible = true;
      
      // 实际项目中，可能需要额外调用 API 获取完整的提交详情
      /*
      getSubmissionDetail(submission.id)
        .then(response => {
          if (response.status === 'ok') {
            this.currentSubmission = response.submission;
            this.submissionDialogVisible = true;
          } else {
            this.$message.error(response.message || '获取提交详情失败');
          }
        })
        .catch(error => {
          console.error('获取提交详情失败:', error);
          this.$message.error('获取提交详情失败，请稍后再试');
        });
      */
    },
    
    // 重新提交代码
    resubmit(submission) {
      // 实际项目中，应该调用 API 重新提交代码
      this.$confirm('确定要重新提交这段代码吗?', '提示', {
        confirmButtonText: '确定',
        cancelButtonText: '取消',
        type: 'warning'
      }).then(() => {
        // 模拟 API 调用
        this.$message.success('重新提交成功，正在评测');
        
        // 更新提交状态
        const index = this.submissions.findIndex(s => s.id === submission.id);
        if (index !== -1) {
          this.submissions[index].status = '评测中';
          if (this.currentSubmission && this.currentSubmission.id === submission.id) {
            this.currentSubmission.status = '评测中';
          }
        }
        
        // 实际 API 调用方式：
        /*
        resubmitCode(submission.id)
          .then(response => {
            if (response.status === 'ok') {
              this.$message.success('重新提交成功，正在评测');
              this.fetchSubmissions();
            } else {
              this.$message.error(response.message || '重新提交失败');
            }
          })
          .catch(error => {
            console.error('重新提交失败:', error);
            this.$message.error('重新提交失败，请稍后再试');
          });
        */
      }).catch(() => {
        // 用户取消操作
      });
    },
    
    // 复制代码
    copyCode() {
      if (this.currentSubmission && this.currentSubmission.code) {
        const textArea = document.createElement('textarea');
        textArea.value = this.currentSubmission.code;
        document.body.appendChild(textArea);
        textArea.select();
        document.execCommand('copy');
        document.body.removeChild(textArea);
        this.$message.success('代码已复制到剪贴板');
      }
    },
    
    // 筛选
    handleFilter() {
      this.currentPage = 1;
      this.fetchSubmissions();
    },
    
    // 重置筛选条件
    resetFilter() {
      this.filterForm = {
        problemId: '',
        status: '',
        language: ''
      };
      this.currentPage = 1;
      this.fetchSubmissions();
    },
    
    // 每页数量变化
    handleSizeChange(val) {
      this.pageSize = val;
      this.fetchSubmissions();
    },
    
    // 页码变化
    handleCurrentChange(val) {
      this.currentPage = val;
      this.fetchSubmissions();
    },
    
    // 根据提交状态返回不同的标签类型
    getSubmissionStatusType(status) {
      switch (status) {
        case '通过':
          return 'success';
        case '编译错误':
        case '运行错误':
          return 'danger';
        case '超时':
          return 'warning';
        case '评测中':
          return 'info';
        default:
          return 'info';
      }
    },
    
    // 格式化日期
    formatDate(timestamp) {
      if (!timestamp) return '';
      
      const date = new Date(timestamp * 1000);
      return date.toLocaleString();
    }
  }
};
</script>

<style lang="scss" scoped>
.submission-list-container {
  padding: 20px;
  
  .submission-list-card {
    .card-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
    
    .filter-bar {
      margin-bottom: 20px;
    }
    
    .pagination-container {
      margin-top: 20px;
      text-align: right;
    }
  }
  
  .submission-dialog {
    .submission-detail {
      .detail-row {
        margin-bottom: 10px;
        display: flex;
        align-items: center;
        
        .detail-label {
          font-weight: bold;
          margin-right: 10px;
          width: 80px;
        }
      }
      
      .code-section {
        margin-top: 20px;
        border: 1px solid #dcdfe6;
        border-radius: 4px;
        
        .code-header {
          background-color: #f5f7fa;
          padding: 10px;
          border-bottom: 1px solid #dcdfe6;
          display: flex;
          justify-content: space-between;
          align-items: center;
          
          .code-title {
            font-weight: bold;
          }
        }
        
        .code-content {
          padding: 15px;
          background-color: #fafafa;
          white-space: pre-wrap;
          font-family: "Courier New", Courier, monospace;
          font-size: 14px;
          overflow-x: auto;
          margin: 0;
        }
      }
      
      .error-section {
        margin-top: 20px;
        border: 1px solid #f56c6c;
        border-radius: 4px;
        
        .error-header {
          background-color: #fef0f0;
          padding: 10px;
          border-bottom: 1px solid #f56c6c;
          font-weight: bold;
          color: #f56c6c;
        }
        
        .error-content {
          padding: 15px;
          background-color: #fff;
          white-space: pre-wrap;
          font-family: "Courier New", Courier, monospace;
          font-size: 14px;
          color: #f56c6c;
          overflow-x: auto;
          margin: 0;
        }
      }
    }
  }
}

@media screen and (max-width: 768px) {
  .submission-list-container {
    padding: 10px;
    
    .form-inline {
      .el-form-item {
        margin-right: 0;
        width: 100%;
        
        .el-form-item__label {
          width: 60px;
        }
        
        .el-form-item__content {
          width: calc(100% - 60px);
        }
      }
    }
  }
}
</style> 