/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   do_middle_cmds.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/31 17:51:37 by saalarco          #+#    #+#             */
/*   Updated: 2026/02/02 08:28:48 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

int			prepare_redirs(t_list *redirs, t_shell *sh);
void		safe_close_rd_fds(t_list *redirs);
t_stage_io	*prepare_stage_io(t_stage_type pos, t_list *redirs, int in_fd,
				int *p);
void		free_cmd_struct(void *input);
void		free_shell_child(t_shell *sh);
void		safe_close_p(int *p);

void	special_middle_exit(t_shell *sh, t_cmd *cmd, int in_fd, int *p)
{
	// msh_restore_fds(sh->save_in, sh->save_out, sh->save_err);
	if (p)
		safe_close_p(p);
	safe_close(in_fd);
	msh_print_last_error(sh);
	safe_close_rd_fds((cmd->redirs));
	if (sh->cmds_start)
		free_cmds(sh->cmds_start);
	else
		free_cmd_struct(cmd);
	free_shell_child(sh);
	exit(1);
}

int	do_middle_commands(t_shell *sh, t_cmd *cmd, int *p, int in_fd)
{
	pid_t		pid;
	t_list		*redirs;
	t_stage_io	*rdr_spec;

	pid = fork_wrap();
	if (pid < 0)
		return (safe_close_p(p), safe_close(in_fd), msh_set_error(sh, FORK_OP),
			-1);
	if (pid == 0)
	{
		// fprintf(stderr, "[CHILD-m] PID %d, parent %d, cmd=%s\n",
		// 	getpid(), getppid(), cmd->argv[0]);
		// if (msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err) == -1)
		// 	special_middle_exit(sh, cmd, in_fd, p);
		redirs = cmd->redirs;
		if (prepare_redirs(redirs, sh) == -1)
			special_middle_exit(sh, cmd, in_fd, p);
		rdr_spec = prepare_stage_io(MIDDLE, redirs, in_fd, p);
		if (!rdr_spec)
			special_middle_exit(sh, cmd, in_fd, p);
		cmd->stage_io = rdr_spec;
		msh_exec_stage(sh, cmd, sh->env, p);
	}
	safe_close(in_fd);
	safe_close(p[1]);
	return (0);
}
