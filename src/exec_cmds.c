#include "../include/exec.h"
#include "../include/log.h"
#include "../Libft/include/libft.h"
#include <stdio.h>
#include <stdlib.h>

/*
How can I manage errors from set_here_doc?
set a mapped status and abort if needed.
*/

int	exec_cmds(t_shell *sh, t_list *cmd_first)
{
	int	nstages;

	nstages = ft_lstsize(cmd_first);
	if (nstages < 1)
	{
		MSH_LOG("No commands to execute");
		return (0);
	}
	// create here docs with expanded variables
	if (set_here_doc(sh, cmd_first) == (-1))
	{
		MSH_LOG("Failed to set here_doc");
		sh->last_status = 1;
		msh_print_last_error(sh);
		return (-1);
	}
	if (nstages > 1)
	{
		MSH_LOG("Executing pipeline with %d stages", nstages);
		// TODO: return msh_exec_pipeline(sh, cmd_first, nstages);
		return (1);
	}
	MSH_LOG("Executing simple command");
	// TODO: return msh_exec_simple(sh, (t_cmd*)cmd_first->content, sh->env);
	// unlink here_doc files
	return (1);
}
