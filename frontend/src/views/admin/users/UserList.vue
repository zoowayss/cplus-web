<template>
  <div class="user-management">
    <h1>用户管理</h1>
    
    <!-- 搜索和筛选表单 -->
    <div class="filter-container">
      <el-form :inline="true" :model="listQuery" class="demo-form-inline">
        <el-form-item label="搜索">
          <el-input v-model="listQuery.search" placeholder="用户名/邮箱" @keyup.enter.native="handleFilter" clearable></el-input>
        </el-form-item>
        <el-form-item label="角色">
          <el-select v-model="listQuery.role" placeholder="全部" clearable @change="handleFilter">
            <el-option v-for="item in roleOptions" :key="item.value" :label="item.label" :value="item.value"></el-option>
          </el-select>
        </el-form-item>
        <el-form-item label="状态">
          <el-select v-model="listQuery.status" placeholder="全部" clearable @change="handleFilter">
            <el-option v-for="item in statusOptions" :key="item.value" :label="item.label" :value="item.value"></el-option>
          </el-select>
        </el-form-item>
        <el-form-item>
          <el-button type="primary" @click="handleFilter">搜索</el-button>
          <el-button type="success" @click="handleCreate">新增用户</el-button>
        </el-form-item>
      </el-form>
    </div>
    
    <!-- 用户列表表格 -->
    <el-table
      v-loading="listLoading"
      :data="userList"
      element-loading-text="加载中..."
      border
      fit
      highlight-current-row
      style="width: 100%"
    >
      <el-table-column align="center" label="ID" width="80">
        <template slot-scope="scope">
          <span>{{ scope.row.id }}</span>
        </template>
      </el-table-column>
      
      <el-table-column align="center" label="用户名" width="120">
        <template slot-scope="scope">
          <span>{{ scope.row.username }}</span>
        </template>
      </el-table-column>
      
      <el-table-column align="center" label="邮箱" width="200">
        <template slot-scope="scope">
          <span>{{ scope.row.email }}</span>
        </template>
      </el-table-column>
      
      <el-table-column align="center" label="角色" width="100">
        <template slot-scope="scope">
          <el-tag :type="scope.row.role | roleFilter">
            {{ scope.row.role | roleNameFilter }}
          </el-tag>
        </template>
      </el-table-column>
      
      <el-table-column align="center" label="状态" width="100">
        <template slot-scope="scope">
          <el-tag :type="scope.row.status | statusFilter">
            {{ scope.row.status | statusNameFilter }}
          </el-tag>
        </template>
      </el-table-column>
      
      <el-table-column align="center" label="注册时间" width="180">
        <template slot-scope="scope">
          <span>{{ new Date(scope.row.created_at * 1000).toLocaleString() }}</span>
        </template>
      </el-table-column>
      
      <el-table-column align="center" label="最后登录" width="180">
        <template slot-scope="scope">
          <span>{{ scope.row.last_login > 0 ? new Date(scope.row.last_login * 1000).toLocaleString() : '从未登录' }}</span>
        </template>
      </el-table-column>
      
      <el-table-column align="center" label="操作" fixed="right" width="300">
        <template slot-scope="scope">
          <el-button size="mini" type="primary" @click="handleEdit(scope.row)">编辑</el-button>
          <el-button 
            size="mini" 
            type="success" 
            @click="handleChangeRole(scope.row)" 
            :disabled="scope.row.id === userId"
          >
            更改角色
          </el-button>
          <el-button 
            size="mini" 
            :type="scope.row.status === 0 ? 'warning' : 'info'" 
            @click="handleChangeStatus(scope.row)" 
            :disabled="scope.row.id === userId"
          >
            {{ scope.row.status === 0 ? '禁用' : '启用' }}
          </el-button>
          <el-button 
            size="mini" 
            type="danger" 
            @click="handleDelete(scope.row)" 
            :disabled="scope.row.id === userId"
          >
            删除
          </el-button>
        </template>
      </el-table-column>
    </el-table>
    
    <!-- 分页 -->
    <div class="pagination-container">
      <el-pagination
        background
        @size-change="handleSizeChange"
        @current-change="handleCurrentChange"
        :current-page="listQuery.page"
        :page-sizes="[10, 20, 30, 50]"
        :page-size="listQuery.limit"
        layout="total, sizes, prev, pager, next, jumper"
        :total="total"
      ></el-pagination>
    </div>
    
    <!-- 用户对话框表单 -->
    <el-dialog :title="dialogStatus === 'create' ? '创建用户' : '编辑用户'" :visible.sync="dialogFormVisible">
      <el-form ref="dataForm" :rules="rules" :model="temp" label-position="left" label-width="80px" style="width: 400px; margin-left:50px;">
        <el-form-item label="用户名" prop="username">
          <el-input v-model="temp.username" placeholder="请输入用户名"></el-input>
        </el-form-item>
        
        <el-form-item label="邮箱" prop="email">
          <el-input v-model="temp.email" placeholder="请输入邮箱地址"></el-input>
        </el-form-item>
        
        <el-form-item label="密码" prop="password" v-if="dialogStatus === 'create'">
          <el-input v-model="temp.password" type="password" placeholder="请输入密码"></el-input>
        </el-form-item>
        
        <el-form-item label="角色" prop="role">
          <el-select v-model="temp.role" placeholder="请选择角色">
            <el-option v-for="item in roleOptions" :key="item.value" :label="item.label" :value="item.value"></el-option>
          </el-select>
        </el-form-item>
      </el-form>
      <div slot="footer" class="dialog-footer">
        <el-button @click="dialogFormVisible = false">取消</el-button>
        <el-button type="primary" @click="dialogStatus === 'create' ? createUser() : updateUser()">确认</el-button>
      </div>
    </el-dialog>
    
    <!-- 角色对话框 -->
    <el-dialog title="更改用户角色" :visible.sync="roleDialogVisible">
      <el-form ref="roleForm" :model="roleTemp" label-position="left" label-width="80px" style="width: 400px; margin-left:50px;">
        <el-form-item label="角色" prop="role">
          <el-select v-model="roleTemp.role" placeholder="请选择角色">
            <el-option v-for="item in roleOptions" :key="item.value" :label="item.label" :value="item.value"></el-option>
          </el-select>
        </el-form-item>
      </el-form>
      <div slot="footer" class="dialog-footer">
        <el-button @click="roleDialogVisible = false">取消</el-button>
        <el-button type="primary" @click="confirmChangeRole">确认</el-button>
      </div>
    </el-dialog>
    
    <!-- 重置密码对话框 -->
    <el-dialog title="重置用户密码" :visible.sync="resetPasswordDialogVisible">
      <el-form ref="passwordForm" :rules="passwordRules" :model="passwordTemp" label-position="left" label-width="80px" style="width: 400px; margin-left:50px;">
        <el-form-item label="新密码" prop="new_password">
          <el-input v-model="passwordTemp.new_password" type="password" placeholder="请输入新密码"></el-input>
        </el-form-item>
      </el-form>
      <div slot="footer" class="dialog-footer">
        <el-button @click="resetPasswordDialogVisible = false">取消</el-button>
        <el-button type="primary" @click="confirmResetPassword">确认</el-button>
      </div>
    </el-dialog>
  </div>
</template>

<script>
import { getUserList, createUser, updateUser, deleteUser, changeUserRole, changeUserStatus, resetUserPassword } from '@/api/user'
import { mapGetters } from 'vuex'

export default {
  name: 'UserList',
  filters: {
    roleFilter(role) {
      const roleMap = {
        0: 'info',  // 学生
        1: 'success',  // 教师
        2: 'danger'  // 管理员
      }
      return roleMap[role]
    },
    roleNameFilter(role) {
      const roleMap = {
        0: '学生',
        1: '教师',
        2: '管理员'
      }
      return roleMap[role]
    },
    statusFilter(status) {
      const statusMap = {
        0: 'success',  // 正常
        1: 'warning',  // 禁用
        2: 'danger'  // 已删除
      }
      return statusMap[status]
    },
    statusNameFilter(status) {
      const statusMap = {
        0: '正常',
        1: '禁用',
        2: '已删除'
      }
      return statusMap[status]
    }
  },
  computed: {
    ...mapGetters([
      'userId'
    ])
  },
  data() {
    return {
      userList: [],
      total: 0,
      listLoading: true,
      listQuery: {
        page: 1,
        limit: 10,
        search: '',
        role: '',
        status: ''
      },
      roleOptions: [
        { label: '学生', value: 0 },
        { label: '教师', value: 1 },
        { label: '管理员', value: 2 }
      ],
      statusOptions: [
        { label: '正常', value: 0 },
        { label: '禁用', value: 1 },
        { label: '已删除', value: 2 }
      ],
      dialogFormVisible: false,
      dialogStatus: '',
      temp: {
        id: undefined,
        username: '',
        email: '',
        password: '',
        role: 0
      },
      rules: {
        username: [{ required: true, message: '请输入用户名', trigger: 'blur' }],
        email: [
          { required: true, message: '请输入邮箱地址', trigger: 'blur' },
          { type: 'email', message: '请输入正确的邮箱地址', trigger: 'blur' }
        ],
        password: [{ required: true, message: '请输入密码', trigger: 'blur' }],
        role: [{ required: true, message: '请选择角色', trigger: 'change' }]
      },
      roleDialogVisible: false,
      roleTemp: {
        id: undefined,
        role: 0
      },
      resetPasswordDialogVisible: false,
      passwordTemp: {
        id: undefined,
        new_password: ''
      },
      passwordRules: {
        new_password: [{ required: true, message: '请输入新密码', trigger: 'blur' }]
      }
    }
  },
  created() {
    this.getList()
  },
  methods: {
    getList() {
      this.listLoading = true
      const offset = (this.listQuery.page - 1) * this.listQuery.limit
      
      getUserList(offset, this.listQuery.limit, this.listQuery.search, this.listQuery.role, this.listQuery.status)
        .then(response => {
          this.userList = response.users
          this.total = response.total
          this.listLoading = false
        })
        .catch(() => {
          this.listLoading = false
        })
    },
    handleFilter() {
      this.listQuery.page = 1
      this.getList()
    },
    handleSizeChange(val) {
      this.listQuery.limit = val
      this.getList()
    },
    handleCurrentChange(val) {
      this.listQuery.page = val
      this.getList()
    },
    resetTemp() {
      this.temp = {
        id: undefined,
        username: '',
        email: '',
        password: '',
        role: 0
      }
    },
    handleCreate() {
      this.resetTemp()
      this.dialogStatus = 'create'
      this.dialogFormVisible = true
      this.$nextTick(() => {
        this.$refs['dataForm'].clearValidate()
      })
    },
    createUser() {
      this.$refs['dataForm'].validate((valid) => {
        if (valid) {
          createUser(this.temp.username, this.temp.password, this.temp.email, this.temp.role)
            .then(() => {
              this.dialogFormVisible = false
              this.$notify({
                title: '成功',
                message: '创建用户成功',
                type: 'success',
                duration: 2000
              })
              this.getList()
            })
            .catch(error => {
              this.$notify({
                title: '错误',
                message: error.message || '创建用户失败',
                type: 'error',
                duration: 2000
              })
            })
        }
      })
    },
    handleEdit(row) {
      this.temp = Object.assign({}, row)
      delete this.temp.password
      this.dialogStatus = 'update'
      this.dialogFormVisible = true
      this.$nextTick(() => {
        this.$refs['dataForm'].clearValidate()
      })
    },
    updateUser() {
      this.$refs['dataForm'].validate((valid) => {
        if (valid) {
          const tempData = Object.assign({}, this.temp)
          updateUser(tempData.id, tempData)
            .then(() => {
              this.dialogFormVisible = false
              this.$notify({
                title: '成功',
                message: '更新用户信息成功',
                type: 'success',
                duration: 2000
              })
              // 替换列表中对应的数据
              const index = this.userList.findIndex(v => v.id === tempData.id)
              if (index !== -1) {
                this.userList.splice(index, 1, Object.assign({}, this.userList[index], tempData))
              }
            })
            .catch(error => {
              this.$notify({
                title: '错误',
                message: error.message || '更新用户信息失败',
                type: 'error',
                duration: 2000
              })
            })
        }
      })
    },
    handleChangeRole(row) {
      this.roleTemp = {
        id: row.id,
        role: row.role
      }
      this.roleDialogVisible = true
    },
    confirmChangeRole() {
      changeUserRole(this.roleTemp.id, this.roleTemp.role)
        .then(() => {
          this.roleDialogVisible = false
          this.$notify({
            title: '成功',
            message: '更改用户角色成功',
            type: 'success',
            duration: 2000
          })
          // 替换列表中对应的数据
          const index = this.userList.findIndex(v => v.id === this.roleTemp.id)
          if (index !== -1) {
            this.userList[index].role = this.roleTemp.role
          }
        })
        .catch(error => {
          this.$notify({
            title: '错误',
            message: error.message || '更改用户角色失败',
            type: 'error',
            duration: 2000
          })
        })
    },
    handleChangeStatus(row) {
      const status = row.status === 0 ? 1 : 0 // 切换状态
      this.$confirm(`确定要${status === 0 ? '启用' : '禁用'}该用户吗?`, '提示', {
        confirmButtonText: '确定',
        cancelButtonText: '取消',
        type: 'warning'
      }).then(() => {
        changeUserStatus(row.id, status)
          .then(() => {
            this.$notify({
              title: '成功',
              message: `${status === 0 ? '启用' : '禁用'}用户成功`,
              type: 'success',
              duration: 2000
            })
            // 替换列表中对应的数据
            const index = this.userList.findIndex(v => v.id === row.id)
            if (index !== -1) {
              this.userList[index].status = status
            }
          })
          .catch(error => {
            this.$notify({
              title: '错误',
              message: error.message || `${status === 0 ? '启用' : '禁用'}用户失败`,
              type: 'error',
              duration: 2000
            })
          })
      }).catch(() => {})
    },
    handleDelete(row) {
      this.$confirm('此操作将永久删除该用户, 是否继续?', '提示', {
        confirmButtonText: '确定',
        cancelButtonText: '取消',
        type: 'warning'
      }).then(() => {
        deleteUser(row.id)
          .then(() => {
            this.$notify({
              title: '成功',
              message: '删除用户成功',
              type: 'success',
              duration: 2000
            })
            // 从列表中移除
            const index = this.userList.findIndex(v => v.id === row.id)
            if (index !== -1) {
              this.userList.splice(index, 1)
            }
          })
          .catch(error => {
            this.$notify({
              title: '错误',
              message: error.message || '删除用户失败',
              type: 'error',
              duration: 2000
            })
          })
      }).catch(() => {})
    },
    handleResetPassword(row) {
      this.passwordTemp = {
        id: row.id,
        new_password: ''
      }
      this.resetPasswordDialogVisible = true
      this.$nextTick(() => {
        this.$refs['passwordForm'].clearValidate()
      })
    },
    confirmResetPassword() {
      this.$refs['passwordForm'].validate((valid) => {
        if (valid) {
          resetUserPassword(this.passwordTemp.id, this.passwordTemp.new_password)
            .then(() => {
              this.resetPasswordDialogVisible = false
              this.$notify({
                title: '成功',
                message: '重置用户密码成功',
                type: 'success',
                duration: 2000
              })
            })
            .catch(error => {
              this.$notify({
                title: '错误',
                message: error.message || '重置用户密码失败',
                type: 'error',
                duration: 2000
              })
            })
        }
      })
    }
  }
}
</script>

<style scoped>
.user-management {
  padding: 20px;
}

.filter-container {
  margin-bottom: 20px;
}

.pagination-container {
  margin-top: 20px;
  display: flex;
  justify-content: center;
}
</style> 