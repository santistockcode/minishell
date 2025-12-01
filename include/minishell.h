#ifndef MINISHELL_H
# define MINISHELL_H

#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "../Libft/include/libft.h"

typedef struct s_shell
{
	int i;
} t_shell;


# define SUCCESS 0
# define MALLOC_ERROR -1
# define INPUT_ERROR -2
# define FILE_ERROR -3
# define SYNTAX_ERROR -4
#endif