/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_cmds.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 15:10:05 by saalarco          #+#    #+#             */
/*   Updated: 2026/01/15 18:38:01 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/exec.h"
#include "../include/log.h"
#include "../Libft/include/libft.h"
#include "../include/minishell.h"
#include "../include/syswrap.h"
#include <stdio.h>
#include <stdlib.h>

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
	{
		logger("exec_cmds", "No commands to execute... new phone who this");
		return (0);
	}
	if (set_here_docs(sh, cmd_first) == (-1))
	{
		logger("exec_cmds", "Failed to set here_docs");
		if (exit_status == 130)
			sh->last_status = calculate_status_from_errno(exit_status);
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
			sh->last_status = calculate_status_from_errno(exit_status);
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
