-- 创建数据库
CREATE DATABASE IF NOT EXISTS `cplus` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

USE `cplus`;

-- 用户表
CREATE TABLE IF NOT EXISTS `users` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '用户ID',
  `username` VARCHAR(50) NOT NULL COMMENT '用户名',
  `password` VARCHAR(255) NOT NULL COMMENT '密码（哈希）',
  `email` VARCHAR(100) NOT NULL COMMENT '电子邮件',
  `avatar` VARCHAR(255) DEFAULT NULL COMMENT '头像URL',
  `role` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '角色（0-普通用户，1-管理员）',
  `status` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '状态（0-未激活，1-正常，2-封禁）',
  `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `updated_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  PRIMARY KEY (`id`),
  UNIQUE KEY `username` (`username`),
  UNIQUE KEY `email` (`email`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户表';

-- 题目表
CREATE TABLE IF NOT EXISTS `problems` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '题目ID',
  `title` VARCHAR(100) NOT NULL COMMENT '标题',
  `description` TEXT NOT NULL COMMENT '描述',
  `input_format` TEXT DEFAULT NULL COMMENT '输入格式',
  `output_format` TEXT DEFAULT NULL COMMENT '输出格式',
  `constraints` TEXT DEFAULT NULL COMMENT '约束条件',
  `hint` TEXT DEFAULT NULL COMMENT '提示',
  `time_limit` INT UNSIGNED NOT NULL DEFAULT 1000 COMMENT '时间限制（毫秒）',
  `memory_limit` INT UNSIGNED NOT NULL DEFAULT 65536 COMMENT '内存限制（KB）',
  `difficulty` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '难度（0-简单，1-中等，2-困难）',
  `is_public` TINYINT(1) NOT NULL DEFAULT 1 COMMENT '是否公开',
  `created_by` INT UNSIGNED NOT NULL COMMENT '创建者ID',
  `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `updated_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='题目表';

-- 测试用例表
CREATE TABLE IF NOT EXISTS `test_cases` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '测试用例ID',
  `problem_id` INT UNSIGNED NOT NULL COMMENT '题目ID',
  `input` TEXT NOT NULL COMMENT '输入数据',
  `output` TEXT NOT NULL COMMENT '期望输出',
  `is_sample` TINYINT(1) NOT NULL DEFAULT 0 COMMENT '是否为样例',
  `time_limit` INT UNSIGNED NOT NULL DEFAULT 1000 COMMENT '时间限制（毫秒）',
  `memory_limit` INT UNSIGNED NOT NULL DEFAULT 65536 COMMENT '内存限制（KB）',
  `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  PRIMARY KEY (`id`),
  KEY `problem_id` (`problem_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='测试用例表';

-- 提交记录表
CREATE TABLE IF NOT EXISTS `submissions` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '提交ID',
  `user_id` INT UNSIGNED NOT NULL COMMENT '用户ID',
  `problem_id` INT UNSIGNED NOT NULL COMMENT '题目ID',
  `source_code` TEXT NOT NULL COMMENT '源代码',
  `language` TINYINT UNSIGNED NOT NULL COMMENT '语言（0-C，1-C++，2-Java，3-Python，4-JavaScript）',
  `result` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '结果（0-等待，1-评测中，2-通过，3-答案错误，4-超时，5-内存超限，6-运行错误，7-编译错误，8-系统错误）',
  `score` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '得分',
  `time_used` INT UNSIGNED DEFAULT NULL COMMENT '耗时（毫秒）',
  `memory_used` INT UNSIGNED DEFAULT NULL COMMENT '内存（KB）',
  `error_message` TEXT DEFAULT NULL COMMENT '错误信息',
  `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '提交时间',
  `judged_at` TIMESTAMP NULL DEFAULT NULL COMMENT '评测时间',
  PRIMARY KEY (`id`),
  KEY `user_id` (`user_id`),
  KEY `problem_id` (`problem_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='提交记录表';

-- 测试点结果表
CREATE TABLE IF NOT EXISTS `test_results` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '测试点结果ID',
  `submission_id` INT UNSIGNED NOT NULL COMMENT '提交ID',
  `test_case_id` INT UNSIGNED NOT NULL COMMENT '测试用例ID',
  `result` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '结果（0-等待，1-评测中，2-通过，3-答案错误，4-超时，5-内存超限，6-运行错误，7-编译错误，8-系统错误）',
  `time_used` INT UNSIGNED DEFAULT NULL COMMENT '耗时（毫秒）',
  `memory_used` INT UNSIGNED DEFAULT NULL COMMENT '内存（KB）',
  `output` TEXT DEFAULT NULL COMMENT '实际输出',
  `error_message` TEXT DEFAULT NULL COMMENT '错误信息',
  PRIMARY KEY (`id`),
  KEY `submission_id` (`submission_id`),
  KEY `test_case_id` (`test_case_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='测试点结果表';

-- 讨论表
CREATE TABLE IF NOT EXISTS `discussions` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '讨论ID',
  `problem_id` INT UNSIGNED DEFAULT NULL COMMENT '题目ID（可选）',
  `user_id` INT UNSIGNED NOT NULL COMMENT '用户ID',
  `title` VARCHAR(100) NOT NULL COMMENT '标题',
  `content` TEXT NOT NULL COMMENT '内容',
  `views` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '浏览量',
  `likes` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '点赞数',
  `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `updated_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  PRIMARY KEY (`id`),
  KEY `problem_id` (`problem_id`),
  KEY `user_id` (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='讨论表';

-- 讨论回复表
CREATE TABLE IF NOT EXISTS `discussion_replies` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '回复ID',
  `discussion_id` INT UNSIGNED NOT NULL COMMENT '讨论ID',
  `user_id` INT UNSIGNED NOT NULL COMMENT '用户ID',
  `parent_id` INT UNSIGNED DEFAULT NULL COMMENT '父回复ID（用于嵌套回复）',
  `content` TEXT NOT NULL COMMENT '内容',
  `likes` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '点赞数',
  `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
  `updated_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  PRIMARY KEY (`id`),
  KEY `discussion_id` (`discussion_id`),
  KEY `user_id` (`user_id`),
  KEY `parent_id` (`parent_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='讨论回复表';

-- 用户排名表（预计算的排名）
CREATE TABLE IF NOT EXISTS `user_rankings` (
  `user_id` INT UNSIGNED NOT NULL COMMENT '用户ID',
  `solved_count` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '解决题目数',
  `total_submissions` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '总提交数',
  `acceptance_rate` FLOAT NOT NULL DEFAULT 0 COMMENT '通过率',
  `score` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT '总分',
  `last_submission_at` TIMESTAMP NULL DEFAULT NULL COMMENT '最后提交时间',
  `updated_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
  PRIMARY KEY (`user_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户排名表';

-- 初始化管理员用户（密码为admin的MD5哈希）
INSERT INTO `users` (`username`, `password`, `email`, `role`, `status`)
VALUES ('admin', '21232f297a57a5a743894a0e4a801fc3', 'admin@example.com', 1, 1); 