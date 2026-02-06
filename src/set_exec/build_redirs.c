/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   build_redirs.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/06 12:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/06 23:38:13 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"
#include "../../include/set_exec.h"

static int	process_file_redir(t_redir *redir, t_io_file *io_file)
{
	redir->type = convert_io_type(io_file->type);
	redir->target = ft_strdup(io_file->filename);
	if (!redir->target)
		return (MALLOC_ERROR);
	redir->fd = 1;
	if (redir->type == R_IN)
		redir->fd = 0;
	redir->quoted = 0;
	return (SUCCESS);
}

static int	process_heredoc_redir(t_redir *redir, t_io_here *io_here)
{
	char	*target;
	int		status;

	redir->type = R_HEREDOC;
	target = ft_strdup(io_here->here_end);
	if (!target)
		return (MALLOC_ERROR);
	redir->quoted = 0;
	if (ft_strchr(target, '\'') || ft_strchr(target, '"'))
		redir->quoted = 1;
	if (redir->quoted)
	{
		status = remove_string_quotes(&target);
		if (status != SUCCESS)
			return (free(target), MALLOC_ERROR);
	}
	redir->target = target;
	redir->fd = 0;
	return (SUCCESS);
}

t_redir	*create_redir(t_io_redirect *io_redir)
{
	t_redir	*redir;
	int		status;

	if (!io_redir)
		return (NULL);
	redir = ft_calloc(1, sizeof(t_redir));
	if (!redir)
		return (NULL);
	if (io_redir->io_file)
		status = process_file_redir(redir, io_redir->io_file);
	else if (io_redir->io_here)
		status = process_heredoc_redir(redir, io_redir->io_here);
	else
		return (free(redir), NULL);
	if (status != SUCCESS)
		return (free(redir), NULL);
	return (redir);
}

static int	add_redirs_from_list(t_list *list, t_list **redirs, int is_prefix)
{
	t_list			*node;
	t_io_redirect	*io_redir;
	t_redir			*redir;
	t_list			*new_node;

	node = list;
	while (node)
	{
		if (is_prefix)
			io_redir = ((t_prefix *)node->content)->io_redirect;
		else
			io_redir = ((t_suffix *)node->content)->io_redirect;
		if (io_redir)
		{
			redir = create_redir(io_redir);
			if (!redir)
				return (MALLOC_ERROR);
			new_node = ft_lstnew(redir);
			if (!new_node)
				return (free(redir->target), free(redir), MALLOC_ERROR);
			ft_lstadd_back(redirs, new_node);
		}
		node = node->next;
	}
	return (SUCCESS);
}

/**
 * build_redirs() - Build redirections list from command structure
 *
 * Extracts all redirections from both prefix and suffix lists,
 * converts them to t_redir format, and returns them as a linked list.
 * Returns NULL if no redirections or on error.
 */
t_list	*build_redirs(t_command *command)
{
	t_list	*redirs;

	if (!command)
		return (NULL);
	redirs = NULL;
	if (command->cmd_prefix)
	{
		if (add_redirs_from_list(command->cmd_prefix, &redirs, 1) != SUCCESS)
		{
			ft_lstclear(&redirs, free);
			return (NULL);
		}
	}
	if (command->cmd_suffix)
	{
		if (add_redirs_from_list(command->cmd_suffix, &redirs, 0) != SUCCESS)
		{
			ft_lstclear(&redirs, free);
			return (NULL);
		}
	}
	return (redirs);
}
