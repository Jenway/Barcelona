include config.mk

NAME := minishell

.PHONY: all clean fclean re

all: $(NAME)

# 链接目标
$(NAME): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# 生成 obj/.../%.o，确保对应目录存在，并产生依赖文件
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# 包含自动生成的依赖，如果不存在则忽略
-include $(DEPS)

clean:
	# 删除所有 object 和依赖文件
	rm -rf $(OBJ_DIR)

fclean: clean
	# 删除可执行文件
	rm -f $(NAME)

re: fclean all