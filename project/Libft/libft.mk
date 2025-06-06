LIBFT_DIR := $(ROOT_DIR)/Libft
LIBFT_INCLUDE := $(LIBFT_DIR)
LIBFT_LIB := $(LIBFT_DIR)/libft.a

.PHONY: build_libft
build_libft:
	$(MAKE) -C $(LIBFT_DIR)

