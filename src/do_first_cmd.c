#include "../include/minishell.h"

int         prepare_redirs(t_list *redirs);
void        safe_close_rd_fds(t_list *redirs);
t_stage_io  *prepare_stage_io(t_stage_type pos, t_list *redirs, int in_fd, int *p);

int do_first_command(t_shell *sh, t_cmd *cmd, int *p)
{
	pid_t pid;
	t_list *redirs;
	t_stage_io *rdr_spec;

	pid = fork_wrap();
	if (pid < 0)
		return (msh_set_error(sh, FORK_OP), -1);
	if (pid == 0) // child process, should exit
	{
		// todo: test and manage dup error
		msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err);
		redirs = cmd->redirs;
		if (prepare_redirs(redirs) == -1)
			return (safe_close_rd_fds(redirs), -1); // fixme: should exit here and restore fds
		rdr_spec = prepare_stage_io(FIRST, redirs, -1, p);
		if (!rdr_spec)
			return (safe_close_rd_fds(redirs), -1); // fixme: should exit here and restore fds
		cmd->stage_io = rdr_spec;
		msh_exec_stage(sh, cmd, sh->env, p);
	}
	safe_close(p[1]);
	return (0);
}