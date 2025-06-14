# Root directory detection
ROOT_DIR := $(realpath ..)
include $(ROOT_DIR)/config.mk

# Test-specific configurations
TEST_SRCS := $(wildcard *.c)
TEST_BINS := $(TEST_SRCS:%.c=%)
TEST_OBJS := $(TEST_SRCS:%.c=$(OBJ_DIR)/%.o)

# Compiler flags for tests (add test-specific flags here)
TEST_CFLAGS := $(CFLAGS) -I$(ROOT_DIR)/include

# FILTEROUT
LINK_OBJS  := $(filter-out $(OBJ_DIR)/shell.o, $(OBJS))

.PHONY: all clean fclean test

all: $(TEST_BINS)

$(OBJ_DIR)/%.o: $(ROOT_DIR)/src/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(TEST_CFLAGS) -c $< -o $@

# Pattern rule for test executables
%: %.c $(LINK_OBJS) | $(OBJ_DIR)
	$(CC) $(TEST_CFLAGS) $< $(LINK_OBJS) -o $@ $(LDFLAGS)

test: all
	@echo "Running all tests..."
	@for test in $(TEST_BINS); do \
		echo "Running $$test..."; \
		./$$test || exit 1; \
	done

clean:
	rm -f $(TEST_BINS) $(TEST_OBJS)

fclean: clean

re: fclean all