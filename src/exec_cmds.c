/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_cmds.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 15:10:05 by saalarco          #+#    #+#             */
/*   Updated: 2026/01/16 13:38:06 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"
extern volatile sig_atomic_t exit_status;

/*
TODO: exec_cmds returns status code, but is already set in sh. 
Discuss with parser part where to set error status.
// FIXME: bad smell exec_cmds and set_here_docs admiten los mismos parámetros,
// esto es por que nos hemos dividido así el trabajo pero en verdad, here_docs debería ir fuera de exec
*/
int	exec_cmds(t_shell *sh, t_list *cmd_first)
{
	int	nstages;

	nstages = ft_lstsize(cmd_first);
	if (nstages < 1)
		return (0);
	logger_ctx(sh, cmd_first, "msh_exec_pipeline", "Entry point");
	if (set_here_docs(sh, cmd_first) == (-1) || exit_status == 130)
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
		logger("exec_cmds", "Executing pipeline with more than 1 stage");
		if (msh_exec_pipeline(sh, cmd_first, nstages) == -1)
		{
			logger("exec_cmds", "Failed to execute pipeline");
			// FIXME: decide proper status code
			sh->last_status = exit_status;
			msh_print_last_error(sh);
			unlink_hds(cmd_first);
			return (1);
		}
		return (1);
	}
	logger("exec_cmds", "Executing simple command");
	msh_exec_simple(sh, (t_cmd*)cmd_first->content, sh->env);
	unlink_hds(cmd_first);
	return (1);
}
