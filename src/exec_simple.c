/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_simple.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/31 18:00:07 by saalarco          #+#    #+#             */
/*   Updated: 2026/02/01 11:30:17 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

// export, unset, exit, cd affect the shell when in simple command
// env, echo, pwd do not affect the shell state

int			prepare_redirs(t_list *redirs, t_shell *sh);
void		safe_close_rd_fds(t_list *redirs);
t_stage_io	*prepare_stage_io(t_stage_type pos, t_list *redirs, int in_fd,
				int *p);
void		stage_exit_print(t_shell *sh, t_cmd *cmd, int *p, int exit_code);
void		safe_close_stage_io(t_stage_io *stage_io);
void		dup2_stage_io(t_shell *sh, t_cmd *cmd, int *p);
int			require_standard_fds(t_shell *sh);

int	dup2_stage_io_parent(t_shell *sh, t_cmd *cmd)
{
	const t_stage_io	*rdr_spec;

	rdr_spec = cmd->stage_io;
	if (rdr_spec && rdr_spec->in_fd != -1)
	{
		if (dup2_wrap(rdr_spec->in_fd, STDIN_FILENO) == -1)
			return (msh_set_error(sh, DUP2_OP), (-1));
		safe_close(rdr_spec->in_fd);
	}
	if (rdr_spec && rdr_spec->out_fd != -1)
	{
		if (dup2_wrap(rdr_spec->out_fd, STDOUT_FILENO) == -1)
			return (msh_set_error(sh, DUP2_OP), (-1));
		safe_close(rdr_spec->out_fd);
	}
	return (0);
}

int	exec_builtin_in_parent(t_shell *sh, t_cmd *cmd)
{
	t_list	*redirs;
	int		status;

	redirs = cmd->redirs;
	if (msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err) == -1)
		return (msh_set_error(sh, DUP_OP), -1);
	if (prepare_redirs(redirs, sh) == -1)
		return (msh_restore_fds(sh->save_in, sh->save_out, sh->save_err),
			safe_close_rd_fds(redirs), (-1));
	cmd->stage_io = prepare_stage_io(LAST, redirs, -1, NULL);
	if (cmd->stage_io == NULL)
		return (msh_restore_fds(sh->save_in, sh->save_out, sh->save_err),
			safe_close_rd_fds(redirs), msh_set_error(sh, MALLOC_OP), (-1));
	if (dup2_stage_io_parent(sh, cmd) == (-1))
		return (safe_close_rd_fds(redirs), safe_close_stage_io(cmd->stage_io),
			msh_restore_fds(sh->save_in, sh->save_out, sh->save_err),
			free(cmd->stage_io), (-1));
	status = exec_builtin(cmd, sh);
	safe_close_rd_fds(redirs);
	safe_close_stage_io(cmd->stage_io);
	free(cmd->stage_io);
	msh_restore_fds(sh->save_in, sh->save_out, sh->save_err);
	return (status);
}

int	run_simple(t_shell *sh, t_cmd *cmd, t_list *env, pid_t *pid)
{
	t_list	*redirs;

	*pid = fork_wrap();
	if (*pid < 0)
		return (msh_set_error(sh, FORK_OP), (-1));
	if (*pid == 0)
	{
		// fprintf(stderr, "[simple] PID %d, parent %d, cmd=%s\n",
		// 	getpid(), getppid(), cmd->argv[0]);
		if (msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err) == -1)
			stage_exit_print(sh, cmd, NULL, EXIT_FAILURE);
		redirs = cmd->redirs;
		if (prepare_redirs(redirs, sh) == -1)
			stage_exit_print(sh, cmd, NULL, EXIT_FAILURE);
		cmd->stage_io = prepare_stage_io(LAST, redirs, -1, NULL);
		if (!cmd->stage_io)
			stage_exit_print(sh, cmd, NULL, EXIT_FAILURE);
		msh_exec_stage(sh, cmd, env, NULL);
		exit(EXIT_FAILURE);
	}
	return (0);
}

// returns -1 on any error not related to builtins (so caller should print)
// if error happened in builtin in parent,
// is builtin who managed print and returns
// correct status
int	msh_exec_simple(t_shell *sh, t_cmd *cmd, t_list *env)
{
	int		result;
	int		status;
	pid_t	pid;

	logger_ctx_simple(sh, cmd, "EXEC_SIMPLE", "[line 772]");
	if (require_standard_fds(sh) == -1)
		return (-1);
	if (is_builtin(cmd->argv[0]))
		return (exec_builtin_in_parent(sh, cmd));
	result = run_simple(sh, cmd, env, &pid);
	if (result == -1)
		return (-1);
	if (waitpid(pid, &status, 0) == -1)
		return (msh_set_error(sh, WAITPID_OP), -1);
	if (WIFEXITED(status))
		return (WEXITSTATUS(status));
	return (result);
}
