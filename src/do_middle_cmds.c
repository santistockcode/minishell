#include "../include/minishell.h"

int     prepare_redirs(t_list *redirs, t_shell *sh);
void    safe_close_rd_fds(t_list *redirs);
t_stage_io  *prepare_stage_io(t_stage_type pos, t_list *redirs, int in_fd, int *p);

int do_middle_commands(t_shell *sh, t_cmd *cmd, int *p, int in_fd)
{
	pid_t pid;
	// int fd;
	t_list *redirs;
	t_stage_io *rdr_spec;


	pid = fork_wrap();
	if (pid < 0)
		return (msh_set_error(sh, FORK_OP), -1);
	if (pid == 0)
	{
		// todo: test and manage dup error
		if (msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err) == -1)
		{
			msh_print_last_error(sh);
			exit(1);
		}
		redirs = cmd->redirs;
		if (prepare_redirs(redirs, sh) == -1)
		{
			msh_restore_fds(sh->save_in, sh->save_out, sh->save_err);
			msh_print_last_error(sh);
			safe_close_rd_fds(redirs);
			exit(1);
		}
		rdr_spec = prepare_stage_io(MIDDLE, redirs, in_fd, p);
		if (!rdr_spec)
		{
			msh_restore_fds(sh->save_in, sh->save_out, sh->save_err);
			msh_print_last_error(sh);
			safe_close_rd_fds(redirs);
			exit(1);
		}
		cmd->stage_io = rdr_spec;
		msh_exec_stage(sh, cmd, sh->env, p);
	}
	safe_close(in_fd);
	safe_close(p[1]);
	return (0);
}
