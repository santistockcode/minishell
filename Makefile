NAME = minishell

# Var

CC = cc
CFLAGS = -Wall -Wextra -Werror -g3
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
exec_cmds \
set_here_doc \
free_cmds \
syswrap \
fetch_hd_from_user \
expand_hd \
expand_hd_utils \
unlink_hds


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
READLINE_LIBS = -lreadline

$(NAME): $(OBJ) $(LIBFT_NAME)
	$(MKDIR) $(BIN_DIR)
	$(CC) $(CFLAGS) $(INCLUDE) $(OBJ) $(LIBFT_NAME) $(READLINE_LIBS) -o $@

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