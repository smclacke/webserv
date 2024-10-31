# **************************************************************************** #
#                                                                              #
#                                                         ::::::::             #
#    Makefile                                           :+:    :+:             #
#                                                      +:+                     #
#    By: smclacke <smclacke@student.codam.nl>         +#+                      #
#                                                    +#+                       #
#    Created: 2024/10/22 13:46:36 by smclacke      #+#    #+#                  #
#    Updated: 2024/10/31 16:48:52 by jde-baai      ########   odam.nl          #
#                                                                              #
# **************************************************************************** #

NAME:= webserv
CC:= c++

FLAGS := -std=c++11
FLAGS += -Wall -Wextra -Werror
FLAGS += -g -fsanitize=address

OBJDIR := objs
TPP_FILES := web.tpp
HEADER_FILES := error.hpp server.hpp web.hpp  webserv.hpp


SRC_FILES := main.cpp					\
			parser/error.cpp			\
			parser/init.cpp				\
			parser/parsLocation.cpp		\
			parser/parsUtils.cpp		\
			parser/verify.cpp			\
			server/epoll.cpp			\
			server/run.cpp				\
			server/server.cpp			\
			server/socket.cpp			\
			server/webserv.cpp			\


SRC := $(addprefix src/, $(SRC_FILES))
HEADER := $(addprefix include/, $(HEADER_FILES) $(TPP_FILES))
OBJS := $(SRC:%.cpp=$(OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS) $(HEADER)
	@printf "\033[1;34m---------\\n MAKE $(NAME) \n---------\\n\033[0m"
	@$(CC) $(FLAGS) -o $(NAME) $(OBJS)

$(OBJS): $(OBJDIR)/%.o : %.cpp $(HEADER)
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -c $< -o $@

run: $(NAME)
	@printf "\033[1;33m---------\\n RUN $(NAME) \n---------\\n\033[0m"
	@./$(NAME) "!"

clean:
	@if [ -d "$(OBJDIR)" ]; then \
		printf "\033[1;34m--------------\n removing OBJ files \n--------------\n\033[0m"; \
		$(RM) -r $(OBJDIR); \
	fi

fclean: clean
	@if [ -f "$(NAME)" ]; then \
		printf "\033[1;34m--------------\n removing $(NAME) \n--------------\n\033[0m"; \
		$(RM) $(NAME); \
	fi


re: fclean all

.PHONY: clean fclean re run all