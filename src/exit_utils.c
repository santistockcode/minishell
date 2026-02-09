/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/25 12:46:33 by saalarco          #+#    #+#             */
/*   Updated: 2026/02/09 21:06:44 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

// fds utils
void	safe_close_rd_fds(t_list *redirs);

// free cmds
void	free_cmd_struct(void *input);

// logger
void	logger_ctx_simple(t_shell *sh, t_cmd *cmd, const char *tag,
			const char *message);

// exit utils 2
int		ebip_exit(t_shell *sh, t_list *redirs);

void	putstr_fd_err(int n, ...);

void	safe_close_p(int *p)
{
	if (p)
	{
		safe_close(p[0]);
		safe_close(p[1]);
	}
}

void	free_env_struct_child(void *env)
{
	if (!env)
		return ;
	free(((t_env *)env)->key);
	free(((t_env *)env)->value);
	free(env);
}

void	free_shell_child(t_shell *sh)
{
	if (!sh)
		return ;
	ft_lstclear(&sh->env, free_env_struct_child);
	if (sh->last_err_op)
		free(sh->last_err_op);
	free(sh);
}

void	safe_close_stage_io(t_stage_io *stage_io)
{
	if (!stage_io)
		return ;
	safe_close_p(&stage_io->in_fd);
	safe_close_p(&stage_io->out_fd);
}

// TODO: function to exit child with no errors
void	stage_exit_print(t_shell *sh, t_cmd *cmd, int *p, int exit_code)
{
	if (p)
		safe_close_p(p);
	safe_close_rd_fds(cmd->redirs);
	free(cmd->stage_io);
	msh_restore_fds(sh->save_in, sh->save_out, sh->save_err);
	if (exit_code == 127 || exit_code == 126)
		putstr_fd_err(2, cmd->argv[0], ": command not found");
	else
		msh_print_last_error(sh);
	if (sh->cmds_start)
	{
		free_cmds(sh->cmds_start);
	}
	else
		free_cmd_struct(cmd);
	free_shell_child(sh);
	logger_open_fds("[exit_utils.c]stage_exit_print",
		"[exit_utils.c]stage_exit_print");
	exit(exit_code);
}
