/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   var_expand.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 18:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/04 00:55:56 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/expand.h"

int	expand_variables(t_shell *shell)
{
	t_list		*cmd_node;
	t_command	*cmd;
	int			status;

	if (!shell)
		return (INPUT_ERROR);
	cmd_node = shell->commands;
	while (cmd_node)
	{
		cmd = (t_command *)cmd_node->content;
		status = expand_command(cmd, shell->env);
		if (status != SUCCESS)
			return (status);
		cmd_node = cmd_node->next;
	}
	return (SUCCESS);
}

int	expand_command(t_command *cmd, t_list *env)
{
	int	status;

	if (!cmd)
		return (INPUT_ERROR);
	if (cmd->cmd_prefix)
	{
		status = expand_prefix_list(cmd->cmd_prefix, env);
		if (status != SUCCESS)
			return (status);
	}
	if (cmd->cmd_suffix)
	{
		status = expand_suffix_list(cmd->cmd_suffix, env);
		if (status != SUCCESS)
			return (status);
	}
	if (cmd->cmd_word)
	{
		status = expand_and_quotes(&cmd->cmd_word, env);
		if (status != SUCCESS)
			return (status);
	}
	return (SUCCESS);
}

int	expand_prefix_list(t_list *prefix_list, t_list *env)
{
	t_list		*node;
	t_prefix	*prefix;
	int			status;

	node = prefix_list;
	while (node)
	{
		prefix = (t_prefix *)node->content;
		if (prefix->io_redirect && prefix->io_redirect->io_file)
		{
			status = expand_and_quotes(&prefix->io_redirect->io_file->filename,
				env);
			if (status != SUCCESS)
				return (status);
		}
		node = node->next;
	}
	return (SUCCESS);
}

int	expand_suffix_list(t_list *suffix_list, t_list *env)
{
	t_list		*node;
	t_suffix	*suffix;
	int			status;

	node = suffix_list;
	while (node)
	{
		suffix = (t_suffix *)node->content;
		if (suffix->io_redirect && suffix->io_redirect->io_file)
		{
			status = expand_and_quotes(&suffix->io_redirect->io_file->filename,
				env);
			if (status != SUCCESS)
				return (status);
		}
		if (suffix->word)
		{
			status = expand_and_quotes(&suffix->word, env);
			if (status != SUCCESS)
				return (status);
		}
		node = node->next;
	}
	return (SUCCESS);
}
