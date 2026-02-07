/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free_parsing.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 20:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/07 23:15:00 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"


/**
 * free_commands() - Free entire command list
 * @commands: List of t_command nodes
 *
 * Recursively frees all commands and their contents
 */
void	free_commands(t_list *commands)
{
	ft_lstclear(&commands, (void (*)(void *))free_command);
}

/**
 * free_command() - Free a single command
 * @cmd: Command to free
 *
 * Frees all fields and the command itself
 */
void	free_command(t_command *cmd)
{
	if (!cmd)
		return ;
	free(cmd->cmd_word);
	cmd->cmd_word = NULL;
	if (cmd->cmd_prefix)
		free_prefixes(cmd->cmd_prefix);
	cmd->cmd_prefix = NULL;
	if (cmd->cmd_suffix)
		free_suffixes(cmd->cmd_suffix);
	cmd->cmd_suffix = NULL;
	free(cmd);
}

static void	free_io_redirect(t_prefix *prefix)
{
	if (!prefix || !prefix->io_redirect)
		return ;
	if (prefix->io_redirect->io_file)
	{
		free(prefix->io_redirect->io_file->filename);
		free(prefix->io_redirect->io_file);
	}
	if (prefix->io_redirect->io_here)
	{
		free(prefix->io_redirect->io_here->filename);
		free(prefix->io_redirect->io_here->here_end);
		free(prefix->io_redirect->io_here);
	}
	free(prefix->io_redirect);
}

static void	delete_prefix(void *content)
{
	t_prefix	*prefix;

	if (!content)
		return ;
	prefix = (t_prefix *)content;
	free(prefix->assignment_word);
	if (prefix->io_redirect)
		free_io_redirect(prefix);
	free(prefix);
}

/**
 * free_prefixes() - Free prefix list
 * @prefix_list: List of t_prefix nodes
 */
void	free_prefixes(t_list *prefix_list)
{
	ft_lstclear(&prefix_list, delete_prefix);
}
