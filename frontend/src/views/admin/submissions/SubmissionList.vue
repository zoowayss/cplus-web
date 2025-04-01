<template>
  <div class="admin-submission-list">
    <h1>提交记录管理</h1>
    
    <!-- 筛选表单 -->
    <div class="filter-container">
      <el-form :inline="true" :model="filters" size="small">
        <el-form-item label="题目ID">
          <el-input v-model="filters.problem_id" placeholder="题目ID" clearable></el-input>
        </el-form-item>
        
        <el-form-item label="用户名">
          <el-input v-model="filters.username" placeholder="用户名" clearable></el-input>
        </el-form-item>
        
        <el-form-item label="评测结果">
          <el-select v-model="filters.result" placeholder="评测结果" clearable>
            <el-option v-for="(label, value) in resultOptions" :key="value" :label="label" :value="value"></el-option>
          </el-select>
        </el-form-item>
        
        <el-form-item label="编程语言">
          <el-select v-model="filters.language" placeholder="编程语言" clearable>
            <el-option v-for="(label, value) in languageOptions" :key="value" :label="label" :value="value"></el-option>
          </el-select>
        </el-form-item>
        
        <el-form-item label="提交时间">
          <el-date-picker
            v-model="timeRange"
            type="daterange"
            range-separator="至"
            start-placeholder="开始日期"
            end-placeholder="结束日期"
            value-format="timestamp"
            :default-time="['00:00:00', '23:59:59']"
            >
          </el-date-picker>
        </el-form-item>
        
        <el-form-item>
          <el-button type="primary" @click="fetchData">筛选</el-button>
          <el-button @click="resetFilters">重置</el-button>
        </el-form-item>
      </el-form>
    </div>
    
    <!-- 提交记录表格 -->
    <el-table
      v-loading="loading"
      :data="submissions"
      border
      stripe
      style="width: 100%">
      <el-table-column prop="id" label="ID" width="80"></el-table-column>
      
      <el-table-column label="用户" width="120">
        <template slot-scope="scope">
          {{ scope.row.username || '未知用户' }}
        </template>
      </el-table-column>
      
      <el-table-column label="题目" width="200">
        <template slot-scope="scope">
          <el-link type="primary" @click="viewProblem(scope.row.problem_id)">
            {{ scope.row.problem_id }}. {{ scope.row.problem_title || '未知题目' }}
          </el-link>
        </template>
      </el-table-column>
      
      <el-table-column label="语言" width="100">
        <template slot-scope="scope">
          <el-tag size="mini" :type="getLanguageTagType(scope.row.language)">
            {{ formatLanguage(scope.row.language) }}
          </el-tag>
        </template>
      </el-table-column>
      
      <el-table-column label="评测结果" width="120">
        <template slot-scope="scope">
          <el-tag size="mini" :type="getStatusTagType(scope.row.result)">
            {{ formatResult(scope.row.result) }}
          </el-tag>
        </template>
      </el-table-column>
      
      <el-table-column label="得分" width="80">
        <template slot-scope="scope">
          {{ scope.row.score || 0 }}分
        </template>
      </el-table-column>
      
      <el-table-column label="执行耗时" width="100">
        <template slot-scope="scope">
          {{ scope.row.time_used || 0 }}ms
        </template>
      </el-table-column>
      
      <el-table-column label="内存消耗" width="100">
        <template slot-scope="scope">
          {{ formatMemory(scope.row.memory_used) }}
        </template>
      </el-table-column>
      
      <el-table-column label="提交时间" width="180">
        <template slot-scope="scope">
          {{ formatDate(scope.row.created_at) }}
        </template>
      </el-table-column>
      
      <el-table-column label="操作" fixed="right" width="150">
        <template slot-scope="scope">
          <el-button size="mini" type="primary" @click="viewDetail(scope.row.id)">查看详情</el-button>
        </template>
      </el-table-column>
    </el-table>
    
    <!-- 分页控件 -->
    <div class="pagination-container">
      <el-pagination
        @size-change="handleSizeChange"
        @current-change="handleCurrentChange"
        :current-page="page"
        :page-sizes="[10, 20, 50, 100]"
        :page-size="pageSize"
        layout="total, sizes, prev, pager, next, jumper"
        :total="total">
      </el-pagination>
    </div>
  </div>
</template>

<script>
import { getSubmissions } from '@/api/submission'
import { formatDate, formatDateTime } from '@/utils/formatter'

export default {
  name: 'AdminSubmissionList',
  data() {
    return {
      submissions: [],
      loading: false,
      page: 1,
      pageSize: 10,
      total: 0,
      timeRange: null,
      filters: {
        problem_id: '',
        username: '',
        result: '',
        language: '',
        start_time: '',
        end_time: ''
      },
      resultOptions: {
        0: '等待评测',
        1: '评测中',
        2: '通过',
        3: '答案错误',
        4: '超时',
        5: '内存超限',
        6: '运行错误',
        7: '编译错误',
        8: '系统错误'
      },
      languageOptions: {
        'cpp': 'C++',
        'c': 'C',
        'java': 'Java',
        'python': 'Python',
        'javascript': 'JavaScript'
      }
    }
  },
  watch: {
    timeRange(val) {
      if (val) {
        this.filters.start_time = val[0]
        this.filters.end_time = val[1]
      } else {
        this.filters.start_time = ''
        this.filters.end_time = ''
      }
    }
  },
  created() {
    this.fetchData()
  },
  methods: {
    fetchData() {
      this.loading = true
      
      // 构建查询参数
      const params = {
        page: this.page,
        limit: this.pageSize
      }
      
      // 添加筛选条件
      if (this.filters.problem_id) params.problem_id = this.filters.problem_id
      if (this.filters.username) params.username = this.filters.username
      if (this.filters.result !== '') params.result = this.filters.result
      if (this.filters.language) params.language = this.filters.language
      if (this.filters.start_time) params.start_time = this.filters.start_time
      if (this.filters.end_time) params.end_time = this.filters.end_time
      
      getSubmissions(params)
        .then(response => {
          console.log('获取提交记录响应:', response)
          // 确保我们正确地从response.data中提取数据
          this.submissions = response.submissions || []
          this.total = response.total || 0
          
          // 如果有筛选条件回显，则更新本地筛选条件
          if (response.filters) {
            // 这里可以根据需要处理回显数据
            console.log('筛选条件回显:', response.data.filters)
          }
        })
        .catch(error => {
          console.error('获取提交记录失败:', error)
          this.$message.error('获取提交记录失败，请稍后重试')
        })
        .finally(() => {
          this.loading = false
        })
    },
    resetFilters() {
      this.filters = {
        problem_id: '',
        username: '',
        result: '',
        language: '',
        start_time: '',
        end_time: ''
      }
      this.timeRange = null
      this.page = 1
      this.fetchData()
    },
    handleSizeChange(val) {
      this.pageSize = val
      this.fetchData()
    },
    handleCurrentChange(val) {
      this.page = val
      this.fetchData()
    },
    viewDetail(id) {
      this.$router.push(`/admin/submissions/${id}`)
    },
    viewProblem(id) {
      this.$router.push(`/admin/problems/edit/${id}`)
    },
    formatResult(result) {
      return this.resultOptions[result] || '未知状态'
    },
    formatLanguage(language) {
      // 处理不同的语言格式（可能是数字索引或字符串）
      if (typeof language === 'number') {
        const languages = ['C', 'C++', 'Java', 'Python', 'JavaScript']
        return languages[language] || '未知语言'
      }
      return this.languageOptions[language] || language || '未知语言'
    },
    formatMemory(memory) {
      if (!memory) return '0 KB'
      if (memory < 1024) return `${memory} KB`
      return `${(memory / 1024).toFixed(2)} MB`
    },
    formatDate(timestamp) {
      return formatDateTime(timestamp)
    },
    getStatusTagType(result) {
      const types = {
        0: 'info',    // 等待评测
        1: 'warning', // 评测中
        2: 'success', // 通过
        3: 'danger',  // 答案错误
        4: 'danger',  // 超时
        5: 'danger',  // 内存超限
        6: 'danger',  // 运行错误
        7: 'danger',  // 编译错误
        8: 'danger'   // 系统错误
      }
      return types[result] || 'info'
    },
    getLanguageTagType(language) {
      // 为不同语言分配不同颜色标签
      const languageNumber = typeof language === 'number' ? language : -1
      const types = {
        0: 'info',      // C
        1: 'primary',   // C++
        2: 'success',   // Java
        3: 'warning',   // Python
        4: 'danger'     // JavaScript
      }
      return types[languageNumber] || 'info'
    }
  }
}
</script>

<style scoped>
.admin-submission-list {
  padding: 20px;
}

.filter-container {
  margin-bottom: 20px;
  padding: 15px;
  background-color: #f5f7fa;
  border-radius: 4px;
}

.pagination-container {
  margin-top: 20px;
  text-align: right;
}
</style> 