/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   do_first_cmd.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/27 19:41:34 by saalarco          #+#    #+#             */
/*   Updated: 2026/01/27 21:03:39 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

int			prepare_redirs(t_list *redirs, t_shell *sh);
void		safe_close_rd_fds(t_list *redirs);
t_stage_io	*prepare_stage_io(
				t_stage_type pos, t_list *redirs, int in_fd, int *p);
void	free_cmd_struct(void *input);
void		free_shell_child(t_shell *sh);
void	safe_close_p(int *p);

int	do_first_command(t_shell *sh, t_cmd *cmd, int *p)
{
	pid_t		pid;
	t_stage_io	*rdr_spec;

	pid = fork_wrap();
	if (pid < 0)
		return (msh_set_error(sh, FORK_OP), -1);
	if (pid == 0)
	{
		if (msh_save_fds(&sh->save_in, &sh->save_out, &sh->save_err) == -1)
			return (msh_print_last_error(sh), exit(1), -1);
		if (prepare_redirs(cmd->redirs, sh) == -1)
			return (msh_restore_fds(sh->save_in, sh->save_out, sh->save_err),
				msh_print_last_error(sh), safe_close_rd_fds(cmd->redirs),
					free_shell_child(sh), free_cmd_struct(cmd), exit(1), (-1));
		rdr_spec = prepare_stage_io(FIRST, cmd->redirs, -1, p);
		if (!rdr_spec)
			return (msh_restore_fds(sh->save_in, sh->save_out, sh->save_err),
				msh_print_last_error(sh), safe_close_rd_fds(cmd->redirs),
				free_shell_child(sh), free_cmd_struct(cmd), exit(1), (-1));
		cmd->stage_io = rdr_spec;
		msh_exec_stage(sh, cmd, sh->env, p);
	}
	safe_close(p[1]);
	return (0);
}
