/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   prepare_redirs.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/31 18:05:31 by saalarco          #+#    #+#             */
/*   Updated: 2026/01/31 18:05:38 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

int	open_or_exit(int *fd, char *target, t_shell *sh, t_redir_type type)
{
	if (type == R_IN)
	{
		*fd = open_wrap(target, O_RDONLY, 0);
		if (*fd == -1)
			return (msh_set_error(sh, OPEN_OP), 0);
	}
	else if (type == R_OUT_APPEND)
	{
		*fd = open_wrap(target, O_WRONLY | O_CREAT | O_APPEND, 0644);
		if (*fd == -1)
			return (msh_set_error(sh, OPEN_OP), 0);
	}
	else if (type == R_OUT_TRUNC)
	{
		*fd = open_wrap(target, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (*fd == -1)
			return (msh_set_error(sh, OPEN_OP), 0);
	}
	return (1);
}

int	prepare_redirs(t_list *redirs, t_shell *sh)
{
	t_redir	*redir;
	t_list	*list_redirs;

	list_redirs = redirs;
	while (list_redirs)
	{
		redir = (t_redir *)list_redirs->content;
		if (redir && redir->type == R_IN)
		{
			if (!open_or_exit(&(redir->fd), redir->target, sh, R_IN))
				return (-1);
		}
		else if (redir && redir->type == R_OUT_APPEND)
		{
			if (!open_or_exit(&(redir->fd), redir->target, sh, R_OUT_APPEND))
				return (-1);
		}
		else if (redir && redir->type == R_OUT_TRUNC)
		{
			if (!open_or_exit(&(redir->fd), redir->target, sh, R_OUT_TRUNC))
				return (-1);
		}
		list_redirs = list_redirs->next;
	}
	return (0);
}
