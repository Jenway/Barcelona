FT_PRINTF_DIR := $(ROOT_DIR)/Ft_printf
FT_PRINTF_INCLUDE := $(FT_PRINTF_DIR)
FT_PRINTF_LIB := $(FT_PRINTF_DIR)/libftprintf.a

.PHONY: build_ft_printf
build_ft_printf:
	$(MAKE) -C $(FT_PRINTF_DIR)

