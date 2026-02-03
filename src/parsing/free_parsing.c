/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free_parsing.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 20:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/03 16:30:37 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"
#include "../../include/parsing.h"

/**
 * free_commands() - Free entire command list
 * @commands: List of t_command nodes
 *
 * Recursively frees all commands and their contents
 */
void	free_commands(t_list *commands)
{
	t_list		*current;
	t_command	*cmd;

	current = commands;
	while (current)
	{
		cmd = (t_command *)current->content;
		if (cmd)
			free_command(cmd);
		current = current->next;
	}
	ft_lstclear(&commands, NULL);
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

/**
 * free_prefixes() - Free prefix list
 * @prefix_list: List of t_prefix nodes
 */
void	free_prefixes(t_list *prefix_list)
{
	t_list		*current;
	t_prefix	*prefix;

	current = prefix_list;
	while (current)
	{
		prefix = (t_prefix *)current->content;
		if (prefix)
		{
			free(prefix->assignment_word);
			prefix->assignment_word = NULL;
			
			if (prefix->io_redirect)
			{
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
			free(prefix);
		}
		current = current->next;
	}
	ft_lstclear(&prefix_list, NULL);
}

/**
 * free_suffixes() - Free suffix list
 * @suffix_list: List of t_suffix nodes
 */
void	free_suffixes(t_list *suffix_list)
{
	t_list		*current;
	t_suffix	*suffix;

	current = suffix_list;
	while (current)
	{
		suffix = (t_suffix *)current->content;
		if (suffix)
		{
			free(suffix->word);
			suffix->word = NULL;
			
			if (suffix->io_redirect)
			{
				if (suffix->io_redirect->io_file)
				{
					free(suffix->io_redirect->io_file->filename);
					free(suffix->io_redirect->io_file);
				}
				if (suffix->io_redirect->io_here)
				{
					free(suffix->io_redirect->io_here->filename);
					free(suffix->io_redirect->io_here->here_end);
					free(suffix->io_redirect->io_here);
				}
				free(suffix->io_redirect);
			}
			free(suffix);
		}
		current = current->next;
	}
	ft_lstclear(&suffix_list, NULL);
}
