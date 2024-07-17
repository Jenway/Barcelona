CC = gcc
FLAGS = -Wall -Wextra -Werror
NAME = libftprintf.a
SRC_DIR = src
OBJ_DIR = obj
TEST_SRC_DIR = test

SRCS := $(wildcard src/*.c)
TEST_SRCS := $(wildcard test/*.c)
OBJS := $(patsubst src/%.c, obj/%.o, $(SRCS))
TEST_OBJS := $(patsubst test/%.c, obj/%.o, $(TEST_SRCS))


all: $(NAME)

$(NAME): $(OBJS) 
	ar rcs $(NAME) $(OBJS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(FLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(FLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

test: $(TEST_OBJS) $(NAME)
	$(CC) $(FLAGS) $(TEST_OBJS) $(NAME) $(LIBFT) -o run_test
	./run_test

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re


