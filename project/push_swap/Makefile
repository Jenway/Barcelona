# === Project Settings ===
NAME        := push_swap
CHECKER     := checker
ROOT_DIR    := ..
SRC_DIR     := src
OBJ_DIR     := obj

SRCS_ALL	:= $(wildcard $(SRC_DIR)/*.c)
SRCS 		:= $(filter-out $(SRC_DIR)/operations.c, $(SRCS_ALL))
OBJS        := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS        := $(OBJS:.o=.d)  # 自动依赖文件（.d）

CC          := cc
CFLAGS    := -Wall -Wextra -Werror -MMD -MP  # 自动依赖相关选项

# === External Dependencies ===
include $(ROOT_DIR)/Libft/libft.mk
include $(ROOT_DIR)/Ft_printf/ft_printf.mk
include $(ROOT_DIR)/Get_next_line/gnl.mk

INCLUDES    := -I$(LIBFT_INCLUDE) -I$(FT_PRINTF_INCLUDE) -I$(GNL_INCLUDE)
LDFLAGS     := $(LIBFT_LIB) $(FT_PRINTF_LIB) $(GNL_LIB)

# === Rules ===
.PHONY: all clean fclean re

all: $(NAME) $(CHECKER)

$(NAME): $(OBJS) $(OBJ_DIR)/push_swap.o $(OBJ_DIR)/operations_print.o | $(OBJ_DIR)
	@echo "🔗 Linking $@..."
	$(CC) $(CFLAGS) $(OBJ_DIR)/push_swap.o $(OBJ_DIR)/operations_print.o $(OBJS) $(LDFLAGS) -o $@

$(CHECKER): $(OBJ_DIR)/checker.o $(OBJS) $(OBJ_DIR)/operations_check.o | $(OBJ_DIR)
	@echo "🔗 Linking $@..."
	$(CC) $(CFLAGS) $(OBJ_DIR)/checker.o $(OBJ_DIR)/operations_check.o $(OBJS) $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "🛠  Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/checker.o: checker.c | $(OBJ_DIR)
	@echo "🛠  Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/push_swap.o: push_swap.c | $(OBJ_DIR)
	@echo "🛠  Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/operations_print.o: $(SRC_DIR)/operations.c | $(OBJ_DIR)
	@echo "🛠  Compiling $< with OP_PRINT..."
	$(CC) $(CFLAGS) -DOP_PRINT $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/operations_check.o: $(SRC_DIR)/operations.c | $(OBJ_DIR)
	@echo "🛠  Compiling $< without OP_PRINT..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $@

# 自动包含生成的依赖文件（-表示忽略第一次不存在的情况）
-include $(DEPS)

# === Utilities ===
clean:
	@echo "🧹 Cleaning object files..."
	@rm -f $(OBJS) $(DEPS)

fclean: clean
	@echo "🧼 Removing binary..."
	@rm -f $(NAME)

re: fclean all
