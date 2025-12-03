#ifndef MINISHELL_H
# define MINISHELL_H

#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "../Libft/include/libft.h"
#include "log.h"

// do we deep copy envp?
// typedef struct s_env
// {
//     char            *key;
//     char            *value;
//     struct s_env    *next;
// }   t_env;


typedef struct s_shell
{
	int i;
	// t_env   	*env;
    // int      last_status; // last $? value
    // int      should_exit; // 1 if shell should exit so you can clean up
} t_shell;


# define SUCCESS 0
# define MALLOC_ERROR -1
# define INPUT_ERROR -2
# define FILE_ERROR -3
# define SYNTAX_ERROR -4
#endif