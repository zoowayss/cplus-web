<template>
  <div class="competition-management-container">
    <el-card class="competition-list-card">
      <div slot="header" class="card-header">
        <span>比赛管理</span>
        <el-button type="primary" size="small" @click="openCompetitionDialog(null)">
          <i class="el-icon-plus"></i> 添加比赛
        </el-button>
      </div>
      
      <!-- 比赛列表表格 -->
      <el-table :data="competitions" border style="width: 100%" v-loading="loading">
        <el-table-column prop="id" label="ID" width="80" />
        <el-table-column prop="title" label="比赛名称" min-width="180" />
        <el-table-column label="题目数量" width="100">
          <template slot-scope="scope">
            {{ scope.row.problems.length }}
          </template>
        </el-table-column>
        <el-table-column label="开始时间" width="180">
          <template slot-scope="scope">
            {{ formatDate(scope.row.startDate) }}
          </template>
        </el-table-column>
        <el-table-column label="结束时间" width="180">
          <template slot-scope="scope">
            {{ formatDate(scope.row.endDate) }}
          </template>
        </el-table-column>
        <el-table-column label="状态" width="100">
          <template slot-scope="scope">
            <el-tag 
              :type="getCompetitionStatusType(scope.row)" 
              size="small">
              {{ getCompetitionStatus(scope.row) }}
            </el-tag>
          </template>
        </el-table-column>
        <el-table-column label="操作" width="200">
          <template slot-scope="scope">
            <el-button type="text" size="small" @click="openCompetitionDialog(scope.row)">
              编辑
            </el-button>
            <el-button type="text" size="small" class="danger-text" @click="confirmDeleteCompetition(scope.row)">
              删除
            </el-button>
          </template>
        </el-table-column>
      </el-table>
      
      <!-- 空数据提示 -->
      <el-empty v-if="competitions.length === 0 && !loading" description="暂无比赛数据"></el-empty>
    </el-card>
    
    <!-- 比赛编辑/创建对话框 -->
    <el-dialog 
      :title="isEdit ? '编辑比赛' : '创建比赛'" 
      :visible.sync="dialogVisible" 
      width="50%"
      @closed="resetForm">
      <el-form :model="competitionForm" :rules="rules" ref="competitionForm" label-width="100px">
        <el-form-item label="比赛名称" prop="title">
          <el-input v-model="competitionForm.title" placeholder="请输入比赛名称"></el-input>
        </el-form-item>
        
        <el-form-item label="比赛描述" prop="description">
          <el-input type="textarea" v-model="competitionForm.description" placeholder="请输入比赛描述" :rows="4"></el-input>
        </el-form-item>
        
        <el-form-item label="开始时间" prop="startDate">
          <el-date-picker
            v-model="competitionForm.startDate"
            type="datetime"
            placeholder="选择开始时间"
            value-format="timestamp"
            style="width: 100%">
          </el-date-picker>
        </el-form-item>
        
        <el-form-item label="结束时间" prop="endDate">
          <el-date-picker
            v-model="competitionForm.endDate"
            type="datetime"
            placeholder="选择结束时间"
            value-format="timestamp"
            style="width: 100%">
          </el-date-picker>
        </el-form-item>
        
        <el-form-item label="选择题目" prop="problems">
          <el-transfer
            v-model="competitionForm.problems"
            :data="allProblems"
            :titles="['可选题目', '已选题目']"
            :button-texts="['移除', '添加']"
            :format="{
              noChecked: '${total}',
              hasChecked: '${checked}/${total}'
            }"
            @change="handleChange"
            filterable
            filter-placeholder="请输入题目名称搜索"
            class="problem-transfer">
            <span slot-scope="{ option }" :title="option.title">
              {{ option.id }}: {{ option.title }} ({{ option.difficulty }})
            </span>
          </el-transfer>
        </el-form-item>
      </el-form>
      <span slot="footer" class="dialog-footer">
        <el-button @click="dialogVisible = false">取消</el-button>
        <el-button type="primary" @click="submitCompetitionForm">确定</el-button>
      </span>
    </el-dialog>
    
    <!-- 删除确认对话框 -->
    <el-dialog
      title="确认删除"
      :visible.sync="deleteDialogVisible"
      width="30%">
      <span>确定要删除比赛 "{{ competitionToDelete ? competitionToDelete.title : '' }}" 吗？此操作不可恢复。</span>
      <span slot="footer" class="dialog-footer">
        <el-button @click="deleteDialogVisible = false">取消</el-button>
        <el-button type="danger" @click="deleteCompetition">确定删除</el-button>
      </span>
    </el-dialog>
  </div>
</template>

<script>
import { getProblems } from '@/api/problem';

export default {
  name: 'CompetitionManagement',
  props: {
    params: {
      type: Object,
      default: () => ({})
    }
  },
  data() {
    return {
      loading: false,
      competitions: [],
      allProblems: [],
      dialogVisible: false,
      deleteDialogVisible: false,
      isEdit: false,
      competitionToDelete: null,
      competitionForm: {
        id: null,
        title: '',
        description: '',
        startDate: '',
        endDate: '',
        problems: []
      },
      rules: {
        title: [
          { required: true, message: '请输入比赛名称', trigger: 'blur' },
          { min: 2, max: 50, message: '长度在 2 到 50 个字符', trigger: 'blur' }
        ],
        description: [
          { required: true, message: '请输入比赛描述', trigger: 'blur' }
        ],
        startDate: [
          { required: true, message: '请选择开始时间', trigger: 'change' }
        ],
        endDate: [
          { required: true, message: '请选择结束时间', trigger: 'change' },
          { 
            validator: (rule, value, callback) => {
              if (value && this.competitionForm.startDate && value <= this.competitionForm.startDate) {
                callback(new Error('结束时间必须晚于开始时间'));
              } else {
                callback();
              }
            }, 
            trigger: 'change' 
          }
        ],
        problems: [
          { 
            type: 'array', 
            required: true, 
            message: '请至少选择一个题目', 
            trigger: 'change',
            validator: (rule, value, callback) => {
              if (value && value.length > 0) {
                callback();
              } else {
                callback(new Error('请至少选择一个题目'));
              }
            }
          }
        ]
      }
    };
  },
  created() {
    this.fetchProblems();
    this.loadCompetitions();
    // 设置定时检查，每分钟检查一次是否有过期比赛
    this.checkExpirationInterval = setInterval(() => {
      this.checkExpiredCompetitions();
    }, 60000); // 每分钟检查一次
    // 初始加载时立即检查一次
    this.checkExpiredCompetitions();
  },
  beforeDestroy() {
    // 组件销毁前清除定时器
    if (this.checkExpirationInterval) {
      clearInterval(this.checkExpirationInterval);
    }
  },
  methods: {
    // 获取所有可选题目
    fetchProblems() {
      this.loading = true;
      getProblems({ limit: 1000 }) // 获取尽可能多的题目
        .then(response => {
          if (response.status === 'ok') {
            this.allProblems = (response.problems || []).map(problem => ({
              key: problem.id,
              label: problem.title,
              id: problem.id,
              title: problem.title,
              difficulty: problem.difficulty
            }));
          } else {
            this.$message.error(response.message || '获取题目列表失败');
          }
        })
        .catch(error => {
          console.error('获取题目列表失败:', error);
          this.$message.error('获取题目列表失败，请稍后再试');
        })
        .finally(() => {
          this.loading = false;
        });
    },
    
    // 从localStorage加载比赛数据
    loadCompetitions() {
      try {
        const competitionsJson = localStorage.getItem('competitions');
        this.competitions = competitionsJson ? JSON.parse(competitionsJson) : [];
      } catch (error) {
        console.error('加载比赛数据失败:', error);
        this.competitions = [];
      }
    },
    
    // 保存比赛数据到localStorage
    saveCompetitions() {
      try {
        localStorage.setItem('competitions', JSON.stringify(this.competitions));
      } catch (error) {
        console.error('保存比赛数据失败:', error);
        this.$message.error('保存比赛数据失败');
      }
    },
    
    // 检查并删除过期比赛
    checkExpiredCompetitions() {
      const now = Date.now();
      const expiredCompetitions = this.competitions.filter(comp => comp.endDate < now);
      
      if (expiredCompetitions.length > 0) {
        // 删除过期比赛
        this.competitions = this.competitions.filter(comp => comp.endDate >= now);
        this.saveCompetitions();
        
        // 通知用户
        if (expiredCompetitions.length === 1) {
          this.$message.info(`比赛"${expiredCompetitions[0].title}"已过期并被自动删除`);
        } else {
          this.$message.info(`${expiredCompetitions.length}场比赛已过期并被自动删除`);
        }
      }
    },
    
    // 打开创建/编辑比赛对话框
    openCompetitionDialog(competition) {
      if (competition) {
        // 编辑现有比赛
        this.isEdit = true;
        this.competitionForm = { ...competition };
      } else {
        // 创建新比赛
        this.isEdit = false;
        this.competitionForm = {
          id: Date.now(), // 使用时间戳作为临时ID
          title: '',
          description: '',
          startDate: Date.now(),
          endDate: Date.now() + 7 * 24 * 60 * 60 * 1000, // 默认一周后结束
          problems: []
        };
      }
      this.dialogVisible = true;
    },
    
    // 提交比赛表单
    submitCompetitionForm() {
      this.$refs.competitionForm.validate(valid => {
        if (valid) {
          if (this.isEdit) {
            // 更新现有比赛
            const index = this.competitions.findIndex(c => c.id === this.competitionForm.id);
            if (index !== -1) {
              this.competitions.splice(index, 1, { ...this.competitionForm });
              this.$message.success('比赛更新成功');
            }
          } else {
            // 添加新比赛
            this.competitions.push({ ...this.competitionForm });
            this.$message.success('比赛创建成功');
          }
          
          // 保存到localStorage
          this.saveCompetitions();
          this.dialogVisible = false;
        } else {
          return false;
        }
      });
    },
    
    // 确认删除比赛
    confirmDeleteCompetition(competition) {
      this.competitionToDelete = competition;
      this.deleteDialogVisible = true;
    },
    
    // 删除比赛
    deleteCompetition() {
      if (this.competitionToDelete) {
        this.competitions = this.competitions.filter(c => c.id !== this.competitionToDelete.id);
        this.saveCompetitions();
        this.$message.success('比赛删除成功');
        this.deleteDialogVisible = false;
        this.competitionToDelete = null;
      }
    },
    
    // 重置表单
    resetForm() {
      if (this.$refs.competitionForm) {
        this.$refs.competitionForm.resetFields();
      }
      this.competitionForm = {
        id: null,
        title: '',
        description: '',
        startDate: '',
        endDate: '',
        problems: []
      };
    },
    
    // 格式化日期
    formatDate(timestamp) {
      if (!timestamp) return '';
      const date = new Date(timestamp);
      return date.toLocaleString('zh-CN', {
        year: 'numeric',
        month: '2-digit',
        day: '2-digit',
        hour: '2-digit',
        minute: '2-digit'
      });
    },
    
    // 获取比赛状态
    getCompetitionStatus(competition) {
      const now = Date.now();
      if (now < competition.startDate) {
        return '未开始';
      } else if (now > competition.endDate) {
        return '已结束';
      } else {
        return '进行中';
      }
    },
    
    // 获取比赛状态对应的类型
    getCompetitionStatusType(competition) {
      const status = this.getCompetitionStatus(competition);
      const typeMap = {
        '未开始': 'info',
        '进行中': 'success',
        '已结束': 'danger'
      };
      return typeMap[status] || 'info';
    },
    
    // 处理题目选择变化
    handleChange(value, direction, movedKeys) {
      console.log('已选题目变化:', value);
    }
  }
}
</script>

<style lang="scss" scoped>
.competition-management-container {
  padding: 0 0 20px 0;
  
  .competition-list-card {
    margin-bottom: 20px;
  }
  
  .card-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
  }
  
  .danger-text {
    color: #F56C6C;
  }
  
  .problem-transfer {
    text-align: left;
    display: block;
    
    ::v-deep .el-transfer-panel {
      width: 40%;
      
      @media screen and (max-width: 768px) {
        width: 100%;
        margin-bottom: 10px;
      }
    }
    
    @media screen and (max-width: 768px) {
      ::v-deep .el-transfer__buttons {
        display: flex;
        justify-content: center;
        margin: 10px 0;
      }
    }
  }
  
  @media screen and (max-width: 768px) {
    ::v-deep .el-form-item__label {
      float: none;
      display: block;
      text-align: left;
      margin-bottom: 8px;
    }
    
    ::v-deep .el-form-item__content {
      margin-left: 0 !important;
    }
  }
}

.el-transfer-panel {
  ::v-deep .el-transfer-panel__item {
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
  }
}
</style> 