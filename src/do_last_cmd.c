#include "../include/minishell.h"

int         prepare_redirs(t_list *redirs);
void        safe_close_rd_fds(t_list *redirs);
t_stage_io  *prepare_stage_io(t_stage_type pos, t_list *redirs, int in_fd, int *p);

int do_last_command(t_shell *sh, t_cmd *cmd, int last_fd)
{
	pid_t pid;
	t_list *redirs;
	int status;
	int *p;
	int wtpd_resp;

	pid = fork_wrap(); // from here, better to just exit
	p = NULL;
	if (pid < 0)
		return (msh_set_error(sh, FORK_OP), -1);
	if (pid == 0)
	{
		// todo: test and manage dup error
		msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
		redirs = cmd->redirs;
		if (prepare_redirs(redirs) == -1)
			return (safe_close_rd_fds(redirs), -1); // fixme: should exit here or not? and should restore fds
		cmd->stage_io = prepare_stage_io(LAST, redirs, last_fd, p);
		if (!cmd->stage_io)
			return (safe_close_rd_fds(redirs), -1); // fixme: should exit here, and should restore fds
		msh_exec_stage(sh, cmd, sh->env, p);
	}
	safe_close(last_fd);
	wtpd_resp = waitpid(pid, &status, 0);
	if (wtpd_resp == -1)
		return (msh_set_error(sh, WAITPID_OP), -1);
	if (WIFEXITED(status))
		return WEXITSTATUS(status);
	return (-1); // unknown error
}
