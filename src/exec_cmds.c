/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_cmds.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 15:10:05 by saalarco          #+#    #+#             */
/*   Updated: 2026/01/31 18:23:14 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

extern volatile sig_atomic_t	g_exit_status;

int	exec_simple_wrapper(t_shell *sh, t_list *cmd_first)
{
	int		last_status;
	t_cmd	*cmd;

	// FIXME: msh_exec_simple won't work on NULL cmd, check before callling
	cmd = (t_cmd *)cmd_first->content;
	logger("exec_cmds", "Executing simple command");
	last_status = msh_exec_simple(sh, cmd, sh->env);
	if (last_status == -1)
	{
		logger("exec_cmds",
			"Failed to execute simple command (not by a builtin failure)");
		sh->last_status = 1;
		msh_print_last_error(sh);
		unlink_hds(cmd_first);
		return (1);
	}
	return (last_status);
}

int	exec_pipeline_wrapper(t_shell *sh, t_list *cmd_first, int nstages)
{
	int	last_pipeline_st;

	sh->last_errno = 0;
	logger("exec_cmds", "Executing pipeline with more than 1 stage");
	// msh_exec_pipeline returns -1 on syscall error (pipeline interrupted)
	last_pipeline_st = msh_exec_pipeline(sh, cmd_first, nstages);
	if (last_pipeline_st == -1) // error
	{
		logger("exec_cmds", "Failed to execute pipeline");
		sh->last_status = 1;
		msh_print_last_error(sh);
		unlink_hds(cmd_first);
		return (1);
	}
	return (last_pipeline_st);
}

/*
Returns the exit status of the last command executed in the pipeline.
// FIXME: bad smell exec_cmds and set_here_docs admiten los mismos parámetros,
// esto es por que nos hemos dividido así el trabajo pero en verdad,
	here_docs debería ir fuera de exec
*/
int	exec_cmds(t_shell *sh, t_list *cmd_first)
{
	int	nstages;

	nstages = ft_lstsize(cmd_first);
	if (nstages < 1)
		return (0);
	logger_ctx(sh, cmd_first, "msh_exec_pipeline", "Entry point");
	if (set_here_docs(sh, cmd_first) == (-1) || g_exit_status == 130)
	{
		logger("exec_cmds", "Failed to set here_docs");
		if (g_exit_status == 130)
			sh->last_status = g_exit_status;
		else
			sh->last_status = 1;
		msh_print_last_error(sh);
		unlink_hds(cmd_first);
		return (1);
	}
	if (nstages > 1)
		return (exec_pipeline_wrapper(sh, cmd_first, nstages));
	return (exec_simple_wrapper(sh, cmd_first));
}
