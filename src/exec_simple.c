#include "../include/minishell.h"

// export, unset, exit, cd affect the shell when in simple command
// env, echo, pwd do not affect the shell state

int         prepare_redirs(t_list *redirs);
void        safe_close_rd_fds(t_list *redirs);
t_stage_io  *prepare_stage_io(t_stage_type pos, t_list *redirs, int in_fd, int *p);
void	stage_exit_print(t_shell *sh, t_cmd *cmd, int *p, int exit_code);
void	safe_close_stage_io(t_stage_io *stage_io);
void dup2_stage_io(t_shell *sh, t_cmd *cmd, int *p);

int	exec_builtin_in_parent(t_shell *sh, t_cmd *cmd)
{
	t_list *redirs;
	int status;

	redirs = cmd->redirs;
	if (msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err) == -1)
		return (msh_set_error(sh, DUP_OP), -1);
	if (prepare_redirs(redirs) == -1)
	{
		msh_restore_fds(sh->save_in, sh->save_out, sh->save_err);
		return (safe_close_rd_fds(redirs), -1);
	}
    cmd->stage_io = prepare_stage_io(LAST, redirs, -1, NULL);
    if (cmd->stage_io == NULL)
    {
        msh_restore_fds(sh->save_in, sh->save_out, sh->save_err);
        safe_close_rd_fds(redirs);
        return (msh_set_error(sh, MALLOC_OP), -1);
    }
    dup2_stage_io(sh, cmd, NULL);
	status = exec_builtin(cmd, sh);
	safe_close_rd_fds(redirs); // TODO: IS THIS CORRECT?
    safe_close_stage_io(cmd->stage_io); // TODO: IS THIS CORRECT?
    free(cmd->stage_io);
	msh_restore_fds(sh->save_in, sh->save_out, sh->save_err);
	return (status);
}

// returns -1 on any error not related to builtins (so caller should print)
int msh_exec_simple(t_shell *sh, t_cmd *cmd, t_list *env)
{
    pid_t	pid;
    int		status;

    if (is_builtin(cmd->argv[0]))
        return (exec_builtin_in_parent(sh, cmd));
    pid = fork_wrap();
    if (pid < 0)
        return (msh_set_error(sh, FORK_OP), (-1));
    if (pid == 0)
    {
        if (msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err) == -1)
            stage_exit_print(sh, cmd, NULL, EXIT_FAILURE);
        if (prepare_redirs(cmd->redirs) == -1)
            stage_exit_print(sh, cmd, NULL, EXIT_FAILURE);
        cmd->stage_io = prepare_stage_io(LAST, cmd->redirs, -1, NULL);
		if (!cmd->stage_io)
			stage_exit_print(sh, cmd, NULL, EXIT_FAILURE);
        msh_exec_stage(sh, cmd, env, NULL);
        exit(EXIT_FAILURE);
    }
    if (waitpid(pid, &status, 0) == -1)
        return (msh_set_error(sh, WAITPID_OP), -1);
    if (WIFEXITED(status))
        return (WEXITSTATUS(status));
    return (-1);
}
