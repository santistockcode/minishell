#include "../include/exec.h"
#include "../include/log.h"
#include "../Libft/include/libft.h"
#include <stdio.h>
#include <stdlib.h>



int exec_cmds(t_shell *sh, t_list *cmd_first)
{
	int nstages;

	sh->last_status = 0; // REMOVE AFTER TESTING

	nstages = ft_lstsize(cmd_first);
	if (nstages < 1) {
		MSH_LOG("No commands to execute");
		return 0;
	}
	// create here docs with expanded variables
	if (set_here_doc(sh, cmd_first) == (-1)) {
		MSH_LOG("Failed to set here_doc");
		return -1;
	}
	if (nstages > 1) {
		MSH_LOG("Executing pipeline with %d stages", nstages);
		// TODO: return msh_exec_pipeline(sh, cmd_first, nstages);
		return 1;
	}
	MSH_LOG("Executing simple command");
	// TODO: return msh_exec_simple(sh, (t_cmd*)cmd_first->content, sh->env);
	return 1;
}



