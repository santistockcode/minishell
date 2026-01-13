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
#include <readline/readline.h>

#include "../Libft/include/libft.h"
#include "log.h"
#include "exec.h"

// can you deep copy envp?
extern volatile sig_atomic_t exit_status;

typedef struct s_env
{
     char            *key;
     char            *value;
}   t_env;


typedef struct s_shell
{
	int i;
	t_list   *env;
    // int      last_status; // last $? value
    // int      should_exit; // 1 if shell should exit so you can clean up
} t_shell;

// INIT
int init_minishell(t_shell **minishell,char **envp);
t_list *init_envp(char **envp);

//SEÑALES
void setup_signal();
void	ft_ctrl_mini(int signal);
void	ft_ctrl_quit(int signal);
void	ft_ctrl_heredoc(int signal);

//PRINT VALUES
void	print_env_list(t_list *env);

# define SUCCESS 0
# define MALLOC_ERROR -1
# define INPUT_ERROR -2
# define FILE_ERROR -3
# define SYNTAX_ERROR -4
#endif