# 2024 Jenway
# 42 Barcelona

CC = gcc
CFLAGS = -Wall -Wextra -Werror
NAME = libft.a
SRC_DIR = src
OBJ_DIR = obj
TEST_SRC_DIR = test
TEST_EXEC = run_tests
SRCS = ft_isalpha.c ft_isdigit.c \
	ft_isalnum.c ft_isascii.c ft_isprint.c ft_strlen.c ft_memset.c\
	ft_bzero.c ft_memcpy.c ft_memmove.c ft_strlcpy.c ft_strlcat.c\
	ft_toupper.c ft_tolower.c ft_strchr.c ft_strrchr.c ft_strncmp.c ft_striteri.c\
	ft_memchr.c ft_memcmp.c ft_strnstr.c ft_atoi.c ft_calloc.c ft_strdup.c\
	ft_substr.c ft_strjoin.c ft_strtrim.c ft_split.c ft_itoa.c \
	ft_strmapi.c ft_putchar_fd.c ft_putstr_fd.c ft_putendl_fd.c ft_putnbr_fd.c
BONUS_SRCS = ft_lstnew.c ft_lstadd_front.c ft_lstsize.c ft_lstlast.c\
	ft_lstadd_back.c ft_lstdelone.c ft_lstclear.c ft_lstiter.c ft_lstmap.c

SRCS := $(addprefix $(SRC_DIR)/, $(SRCS))
BONUS_SRCS := $(addprefix $(SRC_DIR)/, $(BONUS_SRCS))
TEST_SRCS = $(wildcard $(TEST_SRC_DIR)/test_*.c)

OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
BONUS_OBJS := $(BONUS_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TEST_OBJS = $(TEST_SRCS:$(TEST_SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

all: $(NAME) bonus

$(NAME): $(OBJS)
# 使用 ar 命令创建静态库 $(NAME)，包含所有的目标文件 $(OBJS)
	ar rc $(NAME) $(OBJS)
# 使用 ranlib 命令为静态库 $(NAME) 生成索引，以加快链接速度
	ranlib $(NAME)

bonus: $(OBJS) $(BONUS_OBJS)
	ar rc $(NAME) $(OBJS) $(BONUS_OBJS)
	ranlib $(NAME)

# Pattern rule for object files, ensuring the directory exists
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure OBJ_DIR exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# 定义 test 目标
test: $(TEST_EXEC)
	./$(TEST_EXEC)

$(TEST_EXEC): $(TEST_OBJS) $(NAME)
	$(CC) $(CFLAGS) $(TEST_OBJS) -o $@ -L. -lft

# Clean removes object files
clean:
	rm -rf $(OBJ_DIR) $(TEST_EXEC)

# Fclean removes all generated files
fclean: clean
	rm -f $(NAME)

# Rebuilds the project from scratch
re: fclean all

so: $(OBJS) $(BONUS_OBJS)
	gcc -nostartfiles -shared -o libft.so $(OBJS) $(BONUS_OBJS)

# Mark targets as phony
.PHONY: all clean fclean re test bonus $(TEST_EXEC) 
