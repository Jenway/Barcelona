MLX_DIR := $(ROOT_DIR)/FdF/minilibx-linux
MLX_INCLUDE := $(MLX_DIR)
MLX_LIB := $(MLX_DIR)/libmlx.a

.PHONY: build_mlx
build_mlx:
	$(MAKE) -C $(MLX_DIR)
