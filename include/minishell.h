/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/18 18:06:44 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/06 16:41:46 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
#include <stdarg.h>
#include <readline/readline.h>
#include <readline/history.h>
# include <fcntl.h>

# include "../Libft/include/libft.h"
#include "log.h"
#include "env.h"
#include "exec.h"
#include "syswrap.h"
#include "token_struct.h"
#include "lexing.h"
#include "parsing.h"
#include "expand.h"
#include "set_exec.h"


extern volatile sig_atomic_t g_exit_status;

typedef struct s_env
{
     char            *key;
     char            *value;
}   t_env;

typedef struct s_shell
{
	t_list		*env;
	t_lexing	*lexing;
	t_list		*cmds_start; // only used (maybe) for cleanining up, DO NOT USE
	t_list		*exec_cmds;		// Execution input: list of t_cmd
	int			last_status;	// last $? value
	int			should_exit;	// 1 if shell should exit so you can clean up
	char		*last_err_op;	// last operation that caused an error
	int			last_errno;		// last errno value
	int			save_in; // only to be used by exec part
	int			save_out; // only to be used by exec part
	int			save_err; // only to be used by exec part
}			t_shell;



int init_minishell(t_shell **minishell,char **envp);

//SEÑALES
void 	setup_signal();
void	setup_signals_heredoc();
void	ft_ctrl_mini(int signal);
void	ft_ctrl_quit(int signal);
void	ft_ctrl_heredoc(int signal);



# define SUCCESS 0
# define MALLOC_ERROR -1
# define INPUT_ERROR -2
# define FILE_ERROR -3
# define SYNTAX_ERROR -4
#endif