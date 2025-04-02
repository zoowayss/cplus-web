<template>
  <div class="discussion-list-container">
    <div class="discussion-header">
      <h2>题目讨论</h2>
      <el-button type="primary" @click="showCreateDiscussionDialog" :disabled="!isAuthenticated">
        <i class="el-icon-plus"></i> 发起讨论
      </el-button>
    </div>

    <div class="discussion-tips" v-if="!isAuthenticated">
      <el-alert
        title="请先登录再参与讨论"
        type="info"
        :closable="false"
        show-icon>
      </el-alert>
    </div>

    <div class="discussion-content">
      <!-- 讨论列表 -->
      <el-card v-if="discussions.length > 0" class="discussion-card" v-for="item in discussions" :key="item.id" shadow="hover">
        <div class="discussion-item" @click="viewDiscussionDetail(item.id)">
          <div class="discussion-title">
            <h3>{{ item.title }}</h3>
            <div class="discussion-meta">
              <span>浏览: {{ item.views || 0 }}</span>
              <span>点赞: {{ item.likes || 0 }}</span>
              <span>创建于: {{ formatTime(item.created_at) }}</span>
            </div>
          </div>
          <div class="discussion-preview">
            {{ truncateContent(item.content) }}
          </div>
        </div>
      </el-card>

      <!-- 暂无讨论提示 -->
      <el-empty v-if="discussions.length === 0" description="暂无讨论，快来发起第一个讨论吧！"></el-empty>

      <!-- 分页 -->
      <el-pagination
        v-if="discussions.length > 0"
        layout="prev, pager, next"
        :total="total"
        :page-size="limit"
        @current-change="handleCurrentChange"
        class="pagination">
      </el-pagination>
    </div>

    <!-- 创建讨论对话框 -->
    <el-dialog
      title="发起讨论"
      :visible.sync="createDialogVisible"
      width="600px">
      <el-form :model="discussionForm" :rules="discussionRules" ref="discussionForm" label-width="80px">
        <el-form-item label="标题" prop="title">
          <el-input v-model="discussionForm.title" placeholder="请输入讨论标题"></el-input>
        </el-form-item>
        <el-form-item label="内容" prop="content">
          <el-input type="textarea" v-model="discussionForm.content" :rows="10" placeholder="请输入讨论内容，支持Markdown格式"></el-input>
        </el-form-item>
      </el-form>
      <span slot="footer" class="dialog-footer">
        <el-button @click="createDialogVisible = false">取消</el-button>
        <el-button type="primary" @click="submitDiscussion" :loading="submitting">发布</el-button>
      </span>
    </el-dialog>

    <!-- 查看讨论详情对话框 -->
    <el-dialog
      :visible.sync="detailDialogVisible"
      width="800px"
      :title="currentDiscussion.title"
      :fullscreen="false">
      <div v-if="currentDiscussion.id">
        <div class="discussion-detail-header">
          <div class="discussion-meta">
            <span>浏览: {{ currentDiscussion.views || 0 }}</span>
            <span>点赞: {{ currentDiscussion.likes || 0 }}</span>
            <span>创建于: {{ formatTime(currentDiscussion.created_at) }}</span>
            <span v-if="currentDiscussion.updated_at !== currentDiscussion.created_at">
              更新于: {{ formatTime(currentDiscussion.updated_at) }}
            </span>
          </div>
        </div>
        
        <div class="discussion-detail-content">
          {{ currentDiscussion.content }}
        </div>
        
        <el-divider content-position="center">回复</el-divider>
        
        <!-- 回复列表 -->
        <div class="replies-container">
          <div v-if="replies.length > 0">
            <div class="reply-item" v-for="reply in replies" :key="reply.id">
              <div class="reply-header">
                <span>用户ID: {{ reply.user_id }}</span>
                <span>{{ formatTime(reply.created_at) }}</span>
              </div>
              <div class="reply-content">
                {{ reply.content }}
              </div>
              <div class="reply-actions">
                <el-button type="text" @click="showReplyDialog(reply.id)" :disabled="!isAuthenticated">回复</el-button>
                <el-button type="text" v-if="canDelete(reply.user_id)" @click="deleteReply(reply.id)">删除</el-button>
              </div>
              
              <!-- 子回复 -->
              <div class="child-replies" v-if="reply.child_replies && reply.child_replies.length > 0">
                <div class="child-reply" v-for="childReply in reply.child_replies" :key="childReply.id">
                  <div class="reply-header">
                    <span>用户ID: {{ childReply.user_id }}</span>
                    <span>{{ formatTime(childReply.created_at) }}</span>
                  </div>
                  <div class="reply-content">
                    {{ childReply.content }}
                  </div>
                  <div class="reply-actions">
                    <el-button type="text" v-if="canDelete(childReply.user_id)" @click="deleteReply(childReply.id)">删除</el-button>
                  </div>
                </div>
              </div>
            </div>
          </div>
          
          <el-empty v-else description="暂无回复"></el-empty>
        </div>
        
        <!-- 添加回复 -->
        <div class="reply-form">
          <el-input
            type="textarea"
            :rows="4"
            placeholder="请输入您的回复"
            v-model="replyContent"
            :disabled="!isAuthenticated">
          </el-input>
          <el-button type="primary" @click="submitReply" :disabled="!isAuthenticated || !replyContent.trim()" class="reply-btn">提交回复</el-button>
        </div>
      </div>
      <div v-else>
        <el-skeleton :rows="10" animated />
      </div>
    </el-dialog>

    <!-- 回复对话框 -->
    <el-dialog
      title="回复"
      :visible.sync="replyDialogVisible"
      width="500px">
      <el-input
        type="textarea"
        :rows="4"
        placeholder="请输入回复内容"
        v-model="childReplyContent">
      </el-input>
      <span slot="footer" class="dialog-footer">
        <el-button @click="replyDialogVisible = false">取消</el-button>
        <el-button type="primary" @click="submitChildReply" :loading="submitting">提交</el-button>
      </span>
    </el-dialog>
  </div>
</template>

<script>
import { mapGetters } from 'vuex';
import {
  getProblemDiscussions,
  getDiscussionDetail,
  createDiscussion,
  getDiscussionReplies,
  createDiscussionReply,
  deleteDiscussionReply
} from '@/api/discussion';

export default {
  name: 'DiscussionList',
  props: {
    problemId: {
      type: Number,
      required: true
    }
  },
  data() {
    return {
      discussions: [],
      total: 0,
      offset: 0,
      limit: 10,
      loading: false,
      createDialogVisible: false,
      detailDialogVisible: false,
      replyDialogVisible: false,
      discussionForm: {
        title: '',
        content: '',
        problem_id: null
      },
      discussionRules: {
        title: [
          { required: true, message: '请输入讨论标题', trigger: 'blur' },
          { min: 3, max: 100, message: '长度在 3 到 100 个字符', trigger: 'blur' }
        ],
        content: [
          { required: true, message: '请输入讨论内容', trigger: 'blur' },
          { min: 10, message: '内容不能少于 10 个字符', trigger: 'blur' }
        ]
      },
      submitting: false,
      currentDiscussion: {},
      replies: [],
      replyContent: '',
      childReplyContent: '',
      currentParentId: null
    };
  },
  computed: {
    ...mapGetters([
      'isAuthenticated',
      'userId',
      'userRole'
    ])
  },
  created() {
    this.fetchDiscussions();
  },
  methods: {
    // 获取讨论列表
    async fetchDiscussions() {
      try {
        this.loading = true;
        const response = await getProblemDiscussions(this.problemId, {
          offset: this.offset,
          limit: this.limit
        });
        console.log(response);
        
        if (response.status === 'ok') {
          this.discussions = response.discussions || [];
          this.total = response.total || 0;
        } else {
          this.$message.error(response.message || '获取讨论列表失败');
        }
      } catch (error) {
        console.error('获取讨论列表错误', error);
        this.$message.error('获取讨论列表失败');
      } finally {
        this.loading = false;
      }
    },
    
    // 显示创建讨论对话框
    showCreateDiscussionDialog() {
      if (!this.isAuthenticated) {
        this.$message.warning('请先登录后再发起讨论');
        return;
      }
      
      // 重置表单
      this.discussionForm = {
        title: '',
        content: '',
        problem_id: this.problemId
      };
      
      this.createDialogVisible = true;
    },
    
    // 提交讨论
    submitDiscussion() {
      this.$refs.discussionForm.validate(async valid => {
        if (!valid) return;
        
        try {
          this.submitting = true;
          const response = await createDiscussion(this.discussionForm);
          
          if (response.status === 'ok') {
            this.$message.success('发起讨论成功');
            this.createDialogVisible = false;
            // 刷新讨论列表
            this.fetchDiscussions();
          } else {
            this.$message.error(response.message || '发起讨论失败');
          }
        } catch (error) {
          console.error('发起讨论错误', error);
          this.$message.error('发起讨论失败');
        } finally {
          this.submitting = false;
        }
      });
    },
    
    // 查看讨论详情
    async viewDiscussionDetail(discussionId) {
      try {
        // 获取讨论详情
        const response = await getDiscussionDetail(discussionId);
        
        if (response.status === 'ok') {
          this.currentDiscussion = response.discussion || {};
          this.detailDialogVisible = true;
          
          // 获取讨论回复
          this.fetchReplies(discussionId);
        } else {
          this.$message.error(response.message || '获取讨论详情失败');
        }
      } catch (error) {
        console.error('获取讨论详情错误', error);
        this.$message.error('获取讨论详情失败');
      }
    },
    
    // 获取讨论回复
    async fetchReplies(discussionId) {
      try {
        const response = await getDiscussionReplies(discussionId);
        
        if (response.status === 'ok') {
          this.replies = response.replies || [];
        } else {
          this.$message.error(response.message || '获取回复失败');
        }
      } catch (error) {
        console.error('获取回复错误', error);
        this.$message.error('获取回复失败');
      }
    },
    
    // 提交回复
    async submitReply() {
      if (!this.replyContent.trim()) {
        this.$message.warning('请输入回复内容');
        return;
      }
      
      try {
        this.submitting = true;
        const response = await createDiscussionReply(this.currentDiscussion.id, {
          content: this.replyContent
        });
        
        if (response.status === 'ok') {
          this.$message.success('回复成功');
          this.replyContent = '';
          // 刷新回复列表
          this.fetchReplies(this.currentDiscussion.id);
        } else {
          this.$message.error(response.message || '回复失败');
        }
      } catch (error) {
        console.error('回复错误', error);
        this.$message.error('回复失败');
      } finally {
        this.submitting = false;
      }
    },
    
    // 显示回复对话框
    showReplyDialog(parentId) {
      if (!this.isAuthenticated) {
        this.$message.warning('请先登录再回复');
        return;
      }
      
      this.currentParentId = parentId;
      this.childReplyContent = '';
      this.replyDialogVisible = true;
    },
    
    // 提交子回复
    async submitChildReply() {
      if (!this.childReplyContent.trim()) {
        this.$message.warning('请输入回复内容');
        return;
      }
      
      try {
        this.submitting = true;
        const response = await createDiscussionReply(this.currentDiscussion.id, {
          content: this.childReplyContent,
          parent_id: this.currentParentId
        });
        
        if (response.status === 'ok') {
          this.$message.success('回复成功');
          this.replyDialogVisible = false;
          // 刷新回复列表
          this.fetchReplies(this.currentDiscussion.id);
        } else {
          this.$message.error(response.message || '回复失败');
        }
      } catch (error) {
        console.error('回复错误', error);
        this.$message.error('回复失败');
      } finally {
        this.submitting = false;
      }
    },
    
    // 删除回复
    async deleteReply(replyId) {
      try {
        await this.$confirm('确定要删除这条回复吗?', '提示', {
          confirmButtonText: '确定',
          cancelButtonText: '取消',
          type: 'warning'
        });
        
        const response = await deleteDiscussionReply(replyId);
        
        if (response.status === 'ok') {
          this.$message.success('删除回复成功');
          // 刷新回复列表
          this.fetchReplies(this.currentDiscussion.id);
        } else {
          this.$message.error(response.message || '删除回复失败');
        }
      } catch (error) {
        if (error !== 'cancel') {
          console.error('删除回复错误', error);
          this.$message.error('删除回复失败');
        }
      }
    },
    
    // 翻页处理
    handleCurrentChange(page) {
      this.offset = (page - 1) * this.limit;
      this.fetchDiscussions();
    },
    
    // 格式化时间
    formatTime(timestamp) {
      if (!timestamp) return '';
      
      const date = new Date(timestamp * 1000);
      return date.toLocaleString();
    },
    
    // 截断内容
    truncateContent(content) {
      if (!content) return '';
      return content.length > 100 ? content.substring(0, 100) + '...' : content;
    },
    
    // 检查是否有删除权限
    canDelete(userId) {
      return this.userId === userId || this.userRole >= 2;
    }
  }
};
</script>

<style scoped>
.discussion-list-container {
  margin-top: 20px;
}

.discussion-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
}

.discussion-tips {
  margin-bottom: 20px;
}

.discussion-card {
  margin-bottom: 15px;
  cursor: pointer;
  transition: all 0.3s;
}

.discussion-card:hover {
  transform: translateY(-3px);
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
}

.discussion-item {
  padding: 5px;
}

.discussion-title {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
}

.discussion-title h3 {
  margin: 0;
  color: #303133;
}

.discussion-meta {
  color: #909399;
  font-size: 13px;
}

.discussion-meta span {
  margin-left: 10px;
}

.discussion-preview {
  margin-top: 10px;
  color: #606266;
  font-size: 14px;
  line-height: 1.6;
}

.pagination {
  margin-top: 20px;
  text-align: center;
}

.discussion-detail-header {
  margin-bottom: 20px;
}

.discussion-detail-content {
  padding: 10px;
  border-radius: 4px;
  background-color: #f7f7f7;
  min-height: 100px;
  white-space: pre-wrap;
  line-height: 1.8;
}

.replies-container {
  margin-top: 20px;
}

.reply-item {
  padding: 10px;
  border-bottom: 1px solid #eee;
  margin-bottom: 15px;
}

.reply-header {
  display: flex;
  justify-content: space-between;
  color: #909399;
  font-size: 13px;
  margin-bottom: 5px;
}

.reply-content {
  padding: 10px;
  background-color: #f7f7f7;
  border-radius: 4px;
  margin-bottom: 5px;
  white-space: pre-wrap;
  line-height: 1.5;
}

.reply-actions {
  text-align: right;
}

.child-replies {
  margin-left: 30px;
  margin-top: 10px;
}

.child-reply {
  padding: 10px;
  border-left: 2px solid #dcdfe6;
  margin-bottom: 10px;
  background-color: #f9f9f9;
}

.reply-form {
  margin-top: 20px;
}

.reply-btn {
  margin-top: 10px;
  float: right;
}
</style> 