# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/02/06 10:39:34 by saalarco          #+#    #+#              #
#    Updated: 2026/02/06 15:42:01 by saalarco         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = minishell

# Var

CC = cc
CFLAGS = -Wall -Wextra -Werror -g3
LDFLAGS = -lreadline -lncurses
RM = rm
RMFLAGS = -rf
MKDIR = mkdir -p
INCLUDE = -I${INCLUDE_DIR}

# Dir

LIBFT_DIR = Libft/
LIBFT_BIN = Libft/bin/
LIBFT_NAME = $(LIBFT_BIN)libft.a
SRC_DIR = src/
OBJ_DIR = bin/obj/
BIN_DIR = bin/
INCLUDE_DIR = include/

#Files

FILES = \
main \
logger \
exec_cmds \
set_here_docs \
free_cmds \
syswrap \
expand_hd \
expand_hd_utils \
unlink_hds \
exec_errors \
exec_utils \
crtl \
signals \
minishell_init \
exec_stage \
path_utils \
exit_utils \
envp/env_init \
envp/print_env \
envp/free_env \
builtins/export \
builtins/env \
builtins/unset \
builtins/exit \
builtins/echo \
builtins/pwd \
builtins_orq \
fds_utils \
exec_pipeline \
exec_simple \
do_first_cmd \
do_middle_cmds \
do_last_cmd \
prepare_redirs \
prepare_stage_io \
prepare_stage_io_utils


# Files add

SRC = $(addprefix $(SRC_DIR), $(addsuffix .c, $(FILES)))

OBJ = $(addprefix $(OBJ_DIR), $(addsuffix .o, $(FILES)))

# 1st rule
all: $(NAME)

# Debug rule
debug: CFLAGS += -g3 -O0 -DDEBUG
debug: $(NAME)

# Fsanitize rule
fsanitize: CFLAGS += -fsanitize=address
fsanitize: $(NAME)

# Comp bin

$(NAME): $(OBJ) $(LIBFT_NAME)
	$(MKDIR) $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJ) $(LIBFT_NAME) $(LDFLAGS) -o $@

# Comp .O
$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

# Compilar la libft
$(LIBFT_NAME):
	$(MAKE) -C $(LIBFT_DIR)


# clean OBJ
clean:
	$(RM) $(RMFLAGS) $(OBJ_DIR)
	$(MAKE) -C $(LIBFT_DIR) clean
# clean binary OBJ
fclean: clean
	$(RM) $(RMFLAGS) $(BIN_DIR) $(NAME)
	$(MAKE) -C $(LIBFT_DIR) fclean

# Recompilar todo
re: fclean all

.PHONY: all clean fclean re