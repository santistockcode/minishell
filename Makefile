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
tuberiex \
envp/env_init \
envp/print_env \
envp/free_env \
envp/export \
envp/unset\
lexing/lexing\
lexing/ft_vector\
lexing/init_lexing\
lexing/free_lexing\
lexing/automa_lexing\
lexing/assing_lexing


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
	$(CC) $(CFLAGS) $(INCLUDE) $(OBJ) $(LIBFT_NAME) $(LDFLAGS) -o $@

# Comp .O
$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

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