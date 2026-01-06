#ifndef MINISHELL_H
# define MINISHELL_H

# include "../Libft/include/libft.h"
# include "log.h"
# include <stddef.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

// envp deep copy
typedef struct s_env
{
	char	*key;
	char	*value;
}			t_env;

typedef struct s_shell
{
	int		i;
	t_list	*env;
	int		last_status; // last $? value
	int		should_exit; // 1 if shell should exit so you can clean up
}			t_shell;

# define SUCCESS 0
# define MALLOC_ERROR -1
# define INPUT_ERROR -2
# define FILE_ERROR -3
# define SYNTAX_ERROR -4
#endif