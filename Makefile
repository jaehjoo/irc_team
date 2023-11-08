CXX = c++
CXXFLAGS = -std=c++98 -I./include
RM = rm -rf
SRC = main ./source/Server ./source/Client ./source/Channel ./source/CommandHandle ./source/utils
SRCC = $(addsuffix .cpp, $(SRC))
OBJ = $(addsuffix .o, $(SRC))
NAME = ircserv

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $<

clean:
	$(RM) $(OBJ)

fclean:
	make -s clean
	$(RM) $(OBJ) $(NAME)

re:
	make -s fclean
	make -s all

.PHONY: all clean fclean re