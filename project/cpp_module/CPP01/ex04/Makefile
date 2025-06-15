NAME = sed_is_for_losers

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRCS = main.cpp Replacer.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(NAME)
	./$(NAME) example.txt hello bye

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
