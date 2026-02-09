/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/18 18:06:44 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/09 20:39:11 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H
# define _POSIX_C_SOURCE 200809L

# include "../Libft/include/libft.h"
# include "env.h"
# include "exec.h"
# include "token_struct.h"
# include "expand.h"
# include "lexing.h"
# include "log.h"
# include "parsing.h"
# include "set_exec.h"
# include "syswrap.h"
# include <errno.h>
# include <fcntl.h>
# include <readline/history.h>
# include <readline/readline.h>
# include <signal.h>
# include <stdarg.h>
# include <stddef.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/ioctl.h>
# include <sys/stat.h>
# include <sys/wait.h>
# include <unistd.h>

extern volatile sig_atomic_t	g_exit_status;

typedef struct s_env
{
	char						*key;
	char						*value;
}								t_env;

typedef struct s_shell
{
	t_list						*env;
	t_lexing					*lexing;
	t_list						*cmds_start;
	t_list						*pars_cmds;
	t_list						*exec_cmds;
	int							last_status;
	int							should_exit;
	char						*last_err_op;
	int							last_errno;
	int							save_in;
	int							save_out;
	int							save_err;
}								t_shell;

int								init_minishell(t_shell **minishell,
									char **envp);
int								msh_isprint(int c);

// SEÑALES
void							setup_signal(void);
void							setup_signals_heredoc(void);
void							ft_ctrl_mini(int signal);
void							ft_ctrl_quit(int signal);
void							ft_ctrl_heredoc(int signal);

# define SUCCESS 0
# define MALLOC_ERROR -1
# define INPUT_ERROR -2
# define FILE_ERROR -3
# define SYNTAX_ERROR -4
#endif