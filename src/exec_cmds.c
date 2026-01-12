/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_cmds.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 15:10:05 by saalarco          #+#    #+#             */
/*   Updated: 2026/01/12 18:13:01 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/exec.h"
#include "../include/log.h"
#include "../Libft/include/libft.h"
#include "../include/minishell.h"
#include <stdio.h>
#include <stdlib.h>


/*
TODO: exec_cmds returns status code, but is already set in sh. 
Discuss with parser part where to set error status.
// FIXME: bad smell exec_cmds and set_here_docs admiten los mismos parámetros
*/
int	exec_cmds(t_shell *sh, t_list *cmd_first)
{
	int	nstages;

	setup_signal();
	nstages = ft_lstsize(cmd_first);
	if (nstages < 1)
	{
		MSH_LOG("No commands to execute, you sure parser is working ok?");
		return (0);
	}
	if (set_here_docs(sh, cmd_first) == (-1))
	{
		MSH_LOG("Failed to set here_docs");
		if (exit_status == 130) // FIXME: check https://github.com/luna7111/shrapnel repo
			sh->last_status = exit_status;
		else
			sh->last_status = 1; // FIXME: calculate en base a errno
		msh_print_last_error(sh);
		unlink_hds(cmd_first);
		return (1);
	}
	if (nstages > 1)
	{
		MSH_LOG("Executing pipeline with %d stages", nstages);
		// TODO: return msh_exec_pipeline(sh, cmd_first, nstages);
		return (1);
	}
	MSH_LOG("Executing simple command");
	// TODO: return msh_exec_simple(sh, (t_cmd*)cmd_first->content, sh->env);
	unlink_hds(cmd_first);
	return (1);
}
