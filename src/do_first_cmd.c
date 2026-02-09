/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   do_first_cmd.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 19:41:34 by saalarco          #+#    #+#             */
/*   Updated: 2026/02/09 21:25:51 by saalarco         ###   ########.fr       */
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

// signals
void		setup_signals_child(void);

// msh_restore_fds(sh->save_in, sh->save_out, sh->save_err);
void	special_first_exit(t_shell *sh, t_cmd *cmd, int *p)
{
	if (p)
		safe_close_p(p);
	msh_print_last_error(sh);
	safe_close_rd_fds((cmd->redirs));
	if (sh->cmds_start)
		free_cmds(sh->cmds_start);
	else
		free_cmd_struct(cmd);
	free_shell_child(sh);
	logger_open_fds("[do_first_cmd.c]special_first_exit🔥",
		"[do_first_cmd.c]special_first_exit");
	exit(1);
}

int	do_first_command(t_shell *sh, t_cmd *cmd, int *p)
{
	pid_t		pid;
	t_stage_io	*rdr_spec;

	pid = fork();
	if (pid < 0)
		return (safe_close_p(p), msh_set_error(sh, FORK_OP), -1);
	if (pid == 0)
	{
		setup_signals_child();
		cmd->pos = FIRST;
		cmd->prev_in_fd = -1;
		if (prepare_redirs(cmd->redirs, sh) == -1)
			special_first_exit(sh, cmd, p);
		rdr_spec = prepare_stage_io(FIRST, cmd->redirs, -1, p);
		if (!rdr_spec)
			special_first_exit(sh, cmd, p);
		cmd->stage_io = rdr_spec;
		msh_exec_stage(sh, cmd, sh->env, p);
	}
	safe_close(p[1]);
	return (0);
}
