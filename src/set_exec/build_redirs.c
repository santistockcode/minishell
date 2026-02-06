/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   build_redirs.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/06 12:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/06 15:33:53 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"
#include "../../include/set_exec.h"

/**
 * create_redir() - Create a t_redir structure from parsing io_redirect
 *
 * Converts parsing redirection format to execution format.
 * Sets appropriate fd (0 for input, 1 for output).
 */
t_redir	*create_redir(t_io_redirect *io_redir)
{
	t_redir	*redir;

	if (!io_redir)
		return (NULL);
	redir = ft_calloc(1, sizeof(t_redir));
	if (!redir)
		return (NULL);
	if (io_redir->io_file)
	{
		redir->type = convert_io_type(io_redir->io_file->type);
		redir->target = ft_strdup(io_redir->io_file->filename);
		if (!redir->target)
		{
			free(redir);
			return (NULL);
		}
		if (redir->type == R_IN)
			redir->fd = 0;
		else
			redir->fd = 1;
		redir->quoted = 0;
	}
	else if (io_redir->io_here)
	{
		redir->type = R_HEREDOC;
		redir->target = ft_strdup(io_redir->io_here->here_end);
		if (!redir->target)
		{
			free(redir);
			return (NULL);
		}
		redir->fd = 0;
		redir->quoted = 0;
	}
	return (redir);
}

/**
 * add_redir_from_prefix() - Extract redirections from prefix list
 *
 * Iterates through prefix list and adds all io_redirect entries
 * to the redirs list (ignoring assignment_words).
 */
int	add_redir_from_prefix(t_list *prefix_list, t_list **redirs)
{
	t_list		*node;
	t_prefix	*prefix;
	t_redir		*redir;
	t_list		*new_node;

	node = prefix_list;
	while (node)
	{
		prefix = (t_prefix *)node->content;
		if (prefix && prefix->io_redirect)
		{
			redir = create_redir(prefix->io_redirect);
			if (!redir)
				return (MALLOC_ERROR);
			new_node = ft_lstnew(redir);
			if (!new_node)
			{
				return (free(redir->target), free(redir), MALLOC_ERROR);
			}
			ft_lstadd_back(redirs, new_node);
		}
		node = node->next;
	}
	return (SUCCESS);
}

/**
 * add_redir_from_suffix() - Extract redirections from suffix list
 *
 * Iterates through suffix list and adds all io_redirect entries
 * to the redirs list (ignoring words).
 */
int	add_redir_from_suffix(t_list *suffix_list, t_list **redirs)
{
	t_list		*node;
	t_suffix	*suffix;
	t_redir		*redir;
	t_list		*new_node;

	node = suffix_list;
	while (node)
	{
		suffix = (t_suffix *)node->content;
		if (suffix && suffix->io_redirect)
		{
			redir = create_redir(suffix->io_redirect);
			if (!redir)
				return (MALLOC_ERROR);
			new_node = ft_lstnew(redir);
			if (!new_node)
			{
				return (free(redir->target), free(redir), MALLOC_ERROR);
			}
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
		if (add_redir_from_prefix(command->cmd_prefix, &redirs) != SUCCESS)
		{
			ft_lstclear(&redirs, free);
			return (NULL);
		}
	}
	if (command->cmd_suffix)
	{
		if (add_redir_from_suffix(command->cmd_suffix, &redirs) != SUCCESS)
		{
			ft_lstclear(&redirs, free);
			return (NULL);
		}
	}
	return (redirs);
}
