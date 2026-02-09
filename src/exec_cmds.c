/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_cmds.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 15:10:05 by saalarco          #+#    #+#             */
/*   Updated: 2026/02/09 20:12:55 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

extern volatile sig_atomic_t	g_exit_status;

/*
One would think that status 126 and 127 would be exposed, but: 
(.env) c1r6s2% echo < intexistant 
zsh: no such file or directory: intexistant
(.env) c1r6s2% echo $?
1
(.env) c1r6s2% echo "shall not pass" > inex
(.env) c1r6s2% cat inex 
shall not pass
(.env) c1r6s2% chmod 000 inex
(.env) c1r6s2% echo < inex
zsh: permission denied: inex
(.env) c1r6s2% echo $?
1

So returning EXIT_FAILURE is ok
*/
int	exec_simple_wrapper(t_shell *sh, t_list *cmd_first)
{
	int		last_status;
	t_cmd	*cmd;

	sh->cmds_start = cmd_first;
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
		return (EXIT_FAILURE);
	}
	return (last_status);
}
// msh_exec_pipeline returns -1 on syscall error (pipeline interrupted)
int	exec_pipeline_wrapper(t_shell *sh, t_list *cmd_first, int nstages)
{
	int	last_pipeline_st;

	sh->last_errno = 0;
	logger("exec_cmds", "Executing pipeline with more than 1 stage");
	last_pipeline_st = msh_exec_pipeline(sh, cmd_first, nstages);
	if (last_pipeline_st == -1)
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
// devuelve -1 si ha habido un error previo a child o builtin
// en cualquier otro caso devuelve el estado de salida del último comando
*/
int	exec_cmds(t_shell *sh, t_list *cmd_first)
{
	int	nstages;

	nstages = ft_lstsize(cmd_first);
	if (nstages < 1)
		return (0);
	logger_ctx(sh, cmd_first, "msh_exec_pipeline", "Entry point");
	if (set_here_docs(sh, cmd_first) == (-1))
	{
		if (g_exit_status == 130)
			return (unlink_hds(cmd_first), 130);
		else
			sh->last_status = 1;
		unlink_hds(cmd_first);
		return (1);
	}
	if (nstages > 1)
		return (exec_pipeline_wrapper(sh, cmd_first, nstages));
	return (exec_simple_wrapper(sh, cmd_first));
}
