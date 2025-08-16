ROOT_DIR ?= $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

NAME := philo
BONUS_NAME := philo_bonus

CC := gcc
CFLAGS := -g -Wall -Wextra -Werror -I$(ROOT_DIR)/include -MMD -MP

SRC_DIR := $(ROOT_DIR)/src
OBJ_DIR := $(ROOT_DIR)/obj

# 主目标源文件与对象文件
SRCS := $(SRC_DIR)/main.c $(SRC_DIR)/routines.c $(SRC_DIR)/utils.c $(SRC_DIR)/strategy_v1.c $(SRC_DIR)/strategy_v2.c $(SRC_DIR)/strategy_v3.c $(SRC_DIR)/strategy_v4.c
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# bonus 目标源文件与对象文件
BONUS_SRCS := $(SRC_DIR)/philo_bonus.c
BONUS_OBJS := $(BONUS_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
BONUS_DEPS := $(BONUS_OBJS:.o=.d)

.PHONY: all bonus clean fclean re

all: $(NAME)

bonus: $(BONUS_NAME)

# 通用目标规则
$(NAME): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BONUS_NAME): $(BONUS_OBJS)
	$(CC) $(BONUS_OBJS) -o $@ $(LDFLAGS)

# 自动创建 obj 目录并编译
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# 包含依赖文件
-include $(DEPS)
-include $(BONUS_DEPS)

clean:
	@echo "Cleaning object files..."
	rm -rf $(OBJ_DIR)

fclean: clean
	@echo "Cleaning binaries..."
	rm -f $(NAME) $(BONUS_NAME)

re: fclean all
