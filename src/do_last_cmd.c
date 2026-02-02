/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   do_last_cmd.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/31 17:51:22 by saalarco          #+#    #+#             */
/*   Updated: 2026/02/02 08:29:01 by saalarco         ###   ########.fr       */
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

void	special_last_exit(t_shell *sh, t_cmd *cmd, int in_fd)
{
	// msh_restore_fds(sh->save_in, sh->save_out, sh->save_err);
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

int	do_last_command(t_shell *sh, t_cmd *cmd, int last_fd, pid_t *pid)
{
	t_list	*redirs;
	int		*p;

	*pid = fork_wrap();
	p = NULL;
	if (*pid < 0)
		return (safe_close(last_fd), msh_set_error(sh, FORK_OP), -1);
	if (*pid == 0)
	{
		// fprintf(stderr, "[CHILD-l] PID %d, parent %d, cmd=%s\n",
		// 	getpid(), getppid(), cmd->argv[0]);
		// if (msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err) == -1)
		// 	special_last_exit(sh, cmd, last_fd);
		redirs = cmd->redirs;
		if (prepare_redirs(redirs, sh) == -1)
			special_last_exit(sh, cmd, last_fd);
		cmd->stage_io = prepare_stage_io(LAST, redirs, last_fd, p);
		if (!cmd->stage_io)
			special_last_exit(sh, cmd, last_fd);
		msh_exec_stage(sh, cmd, sh->env, p);
	}
	safe_close(last_fd);
	return (0);
}
