GNL_DIR := $(ROOT_DIR)/Get_next_line
GNL_INCLUDE := $(GNL_DIR)
GNL_LIB := $(GNL_DIR)/get_next_line.a

.PHONY: build_gnl
build_gnl:
	$(MAKE) -C $(GNL_DIR)

