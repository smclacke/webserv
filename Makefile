# **************************************************************************** #
#                                                                              #
#                                                         ::::::::             #
#    Makefile                                           :+:    :+:             #
#                                                      +:+                     #
#    By: smclacke <smclacke@student.codam.nl>         +#+                      #
#                                                    +#+                       #
#    Created: 2024/10/22 13:46:36 by smclacke      #+#    #+#                  #
#    Updated: 2025/01/13 17:52:27 by smclacke      ########   odam.nl          #
#                                                                              #
# **************************************************************************** #

NAME:= webserv
CC:= c++

FLAGS := -std=c++20
FLAGS += -Wall -Wextra -Werror -O3
#FLAGS += -g -fsanitize=address

OBJDIR := objs
HEADER_FILES := server.hpp error.hpp httpHandler.hpp httpConstants.hpp socket.hpp webserv.hpp web.hpp epoll.hpp


SRC_FILES := main.cpp					\
			http/cgi.cpp				\
			http/generate.cpp			\
			http/generateUtils.cpp		\
			http/httpHandler.cpp		\
			http/parseHead.cpp				\
			http/parseBody.cpp			\
			http/GET.cpp				\
			http/POST.cpp				\
			http/DELETE.cpp				\
			confParser/error.cpp		\
			confParser/parsLocation.cpp	\
			confParser/parsUtils.cpp	\
			confParser/verify.cpp		\
			server/epoll.cpp			\
			server/epoll_utils.cpp		\
			server/server.cpp			\
			server/serverData.cpp		\
			server/socket.cpp			\
			server/webserv.cpp			\
			utils.cpp					\


SRC := $(addprefix src/, $(SRC_FILES))
HEADER := $(addprefix include/, $(HEADER_FILES))
OBJS := $(SRC:%.cpp=$(OBJDIR)/%.o)

all: $(NAME)

$(NAME): $(OBJS) $(HEADER)
	@printf "\033[1;34m\n---------\\n MAKE $(NAME) \n---------\\n\033[0m"
	@$(CC) $(FLAGS) -o $(NAME) $(OBJS)

$(OBJS): $(OBJDIR)/%.o : %.cpp $(HEADER)
	@mkdir -p $(@D)
	@$(CC) $(FLAGS) -c $< -o $@
	@$(eval COMPILE_COUNT=$(shell echo $$(($(COMPILE_COUNT) + 1))))
	@printf "\r\033[1;32mCompiling... $$(($(COMPILE_COUNT))) of $(words $(SRC_FILES)) files completed.\033[0m"

run: $(NAME)
	@printf "\033[1;33m---------\\n RUN $(NAME) \n---------\\n\033[0m"
	@./$(NAME) "!"

clean:
	@if [ -d "$(OBJDIR)" ]; then \
		printf "\033[1;34m--------------\n removing OBJ files \n--------------\n\033[0m"; \
		$(RM) -r $(OBJDIR); \
	fi

fclean: clean clearLogs
	@if [ -f "$(NAME)" ]; then \
		printf "\033[1;34m--------------\n removing $(NAME) \n--------------\n\033[0m"; \
		$(RM) $(NAME); \
	fi

clearLogs:
	@if [ -d "./logs/" ]; then \
		printf "\033[1;34m--------------\n removing log files \n--------------\n\033[0m"; \
		$(RM) -r ./logs/*; \
	fi

re: fclean all

.PHONY: clean fclean re run all clearLogs