/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   close_unused_child_fds.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/09 21:32:13 by saalarco          #+#    #+#             */
/*   Updated: 2026/02/09 21:33:33 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

void	close_io(t_stage_io *io)
{
	if (io->in_fd != -1 && io->in_fd != STDIN_FILENO)
		safe_close(io->in_fd);
	if (io->out_fd != -1 && io->out_fd != STDOUT_FILENO)
		safe_close(io->out_fd);
}

void	close_unused_child_fds(t_stage_type pos, t_stage_io *io, int *p,
		int prev_in_fd)
{
	if (pos == FIRST)
	{
		if (p)
			safe_close(p[0]);
		if (p && io && io->out_fd == p[1])
			safe_close(p[1]);
	}
	else if (pos == MIDDLE)
	{
		if (p)
			safe_close(p[0]);
		if (prev_in_fd != -1 && prev_in_fd != STDIN_FILENO)
			safe_close(prev_in_fd);
		if (p && io && io->out_fd == p[1])
			safe_close(p[1]);
	}
	else if (pos == LAST)
	{
		if (prev_in_fd != -1 && prev_in_fd != STDIN_FILENO)
			safe_close(prev_in_fd);
	}
	if (io)
		close_io(io);
}

void	close_unused_child_fds_wrapper(t_cmd *cmd, int *p)
{
	int	prev_fd;

	if (cmd->pos == MIDDLE || cmd->pos == LAST)
		prev_fd = cmd->prev_in_fd;
	else
		prev_fd = -1;
	close_unused_child_fds(cmd->pos, cmd->stage_io, p, prev_fd);
}
