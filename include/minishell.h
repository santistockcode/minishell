#ifndef MINISHELL_H
# define MINISHELL_H

#define _POSIX_C_SOURCE 200809L

#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#include "../Libft/include/libft.h"
#include "log.h"

// can you deep copy envp?
// typedef struct s_env
// {
//     char            *key;
//     char            *value;
// }   t_env;
extern int exit_status;


typedef struct s_shell
{
	int i;
	// t_list   *env;
    // int      last_status; // last $? value
    // int      should_exit; // 1 if shell should exit so you can clean up
} t_shell;
void septup_signal();

void	ft_ctrl_mini(int signal);
void	ft_ctrl_quit(int signal);



# define SUCCESS 0
# define MALLOC_ERROR -1
# define INPUT_ERROR -2
# define FILE_ERROR -3
# define SYNTAX_ERROR -4
#endif