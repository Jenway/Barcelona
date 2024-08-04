CC = gcc
CFLAGS = -Wall -Wextra -Werror -D BUFFER_SIZE=32

NAME = get_next_line.a

SRC = get_next_line.c get_next_line_utils.c
OBJ = $(SRC:.c=.o)

all: $(OBJ)
	ar rcs $(NAME) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

bonus:

clean:
	rm -f $(OBJ)

fclean: clean

re: fclean all

.PHONY: all clean fclean re
