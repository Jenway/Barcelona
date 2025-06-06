ROOT_DIR := ..
include $(ROOT_DIR)/Libft/libft.mk
include $(ROOT_DIR)/Ft_printf/ft_printf.mk
include $(ROOT_DIR)/Get_next_line/gnl.mk
include $(ROOT_DIR)/FdF/mlx.mk

CXX := gcc
CXXFLAGS := -I$(LIBFT_INCLUDE) -I$(FT_PRINTF_INCLUDE) -I$(GNL_INCLUDE) -I$(MLX_INCLUDE)

LDFLAGS := -L$(MLX_DIR) -lmlx -lXext -lX11 -lm -lz $(LIBFT_LIB) $(FT_PRINTF_LIB) $(GNL_LIB)

SRCS := $(wildcard src/*.c)
OBJ_DIR := obj
OBJS := $(SRCS:src/%.c=$(OBJ_DIR)/%.o)
TARGET := fdf

.PHONY: all clean fclean re init_submodules

all: init_submodules build_libft build_ft_printf build_gnl $(TARGET)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(TARGET): $(OBJS) | $(OBJ_DIR)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: src/%.c | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)
	$(MAKE) -C $(LIBFT_DIR) clean
	$(MAKE) -C $(FT_PRINTF_DIR) clean
	$(MAKE) -C $(GNL_DIR) clean

fclean: clean
	rm -f $(TARGET)
	$(MAKE) -C $(LIBFT_DIR) fclean
	$(MAKE) -C $(FT_PRINTF_DIR) fclean
	$(MAKE) -C $(GNL_DIR) fclean

re: fclean all

# 初始化 Git 子模块（MiniLibX）
init_submodules:
	@if [ ! -d "minilibx-linux/.git" ]; then \
		echo ">> Initializing Git submodules..."; \
		git submodule update --init --recursive; \
	else \
		echo ">> Submodule already initialized."; \
	fi
