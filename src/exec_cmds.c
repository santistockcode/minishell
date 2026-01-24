/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_cmds.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 15:10:05 by saalarco          #+#    #+#             */
/*   Updated: 2026/01/23 15:59:18 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
extern volatile sig_atomic_t exit_status;

/*
Returns the exit status of the last command executed in the pipeline.
// FIXME: bad smell exec_cmds and set_here_docs admiten los mismos parámetros,
// esto es por que nos hemos dividido así el trabajo pero en verdad, here_docs debería ir fuera de exec
*/
int	exec_cmds(t_shell *sh, t_list *cmd_first)
{
	int	nstages;
	int last_pipeline_st;

	nstages = ft_lstsize(cmd_first);
	if (nstages < 1)
		return (0);
	logger_ctx(sh, cmd_first, "msh_exec_pipeline", "Entry point");
	if (set_here_docs(sh, cmd_first) == (-1) || exit_status == 130) // error
	{
		logger("exec_cmds", "Failed to set here_docs");
		if (exit_status == 130)
			sh->last_status = exit_status;
		else
			sh->last_status = 1;
		msh_print_last_error(sh);
		unlink_hds(cmd_first);
		return (1);
	}
	if (nstages > 1)
	{
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
	logger("exec_cmds", "Executing simple command");
	last_pipeline_st = msh_exec_simple(sh, (t_cmd*)cmd_first->content, sh->env);
	unlink_hds(cmd_first);
	return (last_pipeline_st);
}
