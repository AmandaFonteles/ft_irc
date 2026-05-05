### PROGRAM NAME ###
NAME		:= ircserv

### UTILS ###
CC			:= c++
CFLAGS		:= -Wall -Werror -Wextra
#DEBUG_FLAGS	:= -g -g3
DEPS_FLAGS	:= -MMD -MP
#MAKE_FLAGS	:=
PROG_FLAGS	:= -std=c++98
#TEST_FLAGS	:=
RM			:= rm -rf

### DIRECTORIES ###
SRCS_DIR	:= sources
INCLDS_DIR	:= includes
OBJS_DIR	:= objects

### FILES ###
SRCS		:= $(addprefix $(SRCS_DIR)/, \
				parser.cpp \
				Server.cpp \
				main.cpp \
				)
INCLUDES	:= $(INCLDS_DIR)
INCLDS_FLAGS	:= $(addprefix -I , $(INCLUDES))
OBJS		:= $(patsubst $(SRCS_DIR)/%.cpp,$(OBJS_DIR)/%.o,$(SRCS))
DEPS		:= $(patsubst $(SRCS_DIR)/%.cpp,$(OBJS_DIR)/%.d,$(SRCS))

### COLORS ###
DEFAULT		:= \033[0m
BLACK		:= \033[0;30m
RED			:= \033[0;31m
GREEN		:= \033[0;32m
UGREEN		:= \033[4;32m
YELLOW		:= \033[;33m
BLUE		:= \033[0;34m
PURPLE		:= \033[0;35m
CYAN		:= \033[0;36m
BWHITE		:= \033[1;37m
NEW			:= \r\033[K

### PROJECT ###
all: $(NAME)

$(NAME): $(OBJS)
	@printf "$(NEW)$(PURPLE)[$(NAME)] $(UGREEN)Building:$(DEFAULT)$(BWHITE) $@$(DEFAULT)"
	@$(CC) $(CFLAGS) $(OBJS) $(INCLDS_FLAGS) $(PROG_FLAGS) $(TEST_FLAGS) -o $@
	@printf "\n"

-include $(DEPS)
$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp
	@printf "$(NEW)$(PURPLE)[$(NAME)] $(UGREEN)Building:$(DEFAULT) $<"
	@mkdir -p $(OBJS_DIR)
	@$(CC) $(DEPS_FLAGS) $(CFLAGS) $(INCLDS_FLAGS) $(PROG_FLAGS) $(TEST_FLAGS) -c $< -o $@

clean:
	@printf "$(PURPLE)[$(NAME)] $(RED)Removing $(DEFAULT)$(OBJS_DIR) files\n"
	@$(RM) $(OBJS_DIR)

fclean: clean
	@printf "$(PURPLE)[$(NAME)] $(RED)Removing $(DEFAULT)$(NAME)\n"
	@$(RM) $(NAME)

re: fclean all

cre:
	@clear
	@make -s re

.PHONY: all clean fclean re cre
