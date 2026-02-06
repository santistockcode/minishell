/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   prepare_stage_io_utils.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/31 18:05:43 by saalarco          #+#    #+#             */
/*   Updated: 2026/02/06 14:44:10 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"


int	get_r_in_redir_fd(t_list *redirs)
{
	t_redir	*redir;
	int		last_fd;

	last_fd = -1;
	while (redirs)
	{
		redir = (t_redir *)redirs->content;
		if (redir && redir->type == R_IN)
			last_fd = redir->fd;
		redirs = redirs->next;
	}
	return (last_fd);
}

// it fetches the last output redir 
// (example: "cat < infile > outfile1 > outfile2" should write to outfile2)
int	get_r_out_redir_fd(t_list *redirs)
{
	t_redir	*redir;

	while (redirs)
	{
		redir = (t_redir *)redirs->content;
		if (redir && (redir->type == R_OUT_TRUNC || redir->type == R_OUT_APPEND)
			&& redirs->next == NULL)
			return (redir->fd);
		redirs = redirs->next;
	}
	return (-1);
}

int	get_r_out_mode(t_list *redirs)
{
	t_redir	*redir;

	while (redirs)
	{
		redir = (t_redir *)redirs->content;
		if (redir && (redir->type == R_OUT_TRUNC || redir->type == R_OUT_APPEND)
			&& redirs->next == NULL)
		{
			if (redir->type == R_OUT_TRUNC)
				return (OM_TRUNC);
			else if (redir->type == R_OUT_APPEND)
				return (OM_APPEND);
		}
		redirs = redirs->next;
	}
	return (-1);
}
