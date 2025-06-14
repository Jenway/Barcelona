ROOT_DIR ?= $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

CC := gcc
CFLAGS := -Wall -Wextra -I$(ROOT_DIR)/include -MMD -MP -DRELEASE
LDFLAGS := -lreadline -ltinfo -lncurses

# 源码和目标目录
SRC_DIR := $(ROOT_DIR)/src
OBJ_DIR := $(ROOT_DIR)/obj

# 递归查找所有 .c 文件
SRCS := $(shell find $(SRC_DIR) -type f -name '*.c')

# 将 src/.../xxx.c 转换为 obj/.../xxx.o
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# 自动生成的依赖文件列表（.d）
DEPS := $(OBJS:.o=.d)
