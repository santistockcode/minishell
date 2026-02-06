/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   build_argv.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mario <mario@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/06 12:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/06 16:34:56 by mario            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"
#include "../../include/set_exec.h"

/**
 * count_argv_size() - Count total arguments in command
 *
 * Counts cmd_word (if present) + all word suffixes (excluding redirections).
 */
int	count_argv_size(t_command *command)
{
	int			count;
	t_list		*node;
	t_suffix	*suffix;

	count = 0;
	if (command->cmd_word)
		count++;
	node = command->cmd_suffix;
	while (node)
	{
		suffix = (t_suffix *)node->content;
		if (suffix && suffix->word)
			count++;
		node = node->next;
	}
	return (count);
}

/**
 * build_argv() - Build argv array from command structure
 *
 * Creates a NULL-terminated array containing:
 * 1. cmd_word (if present)
 * 2. All words from suffix list (excluding redirections)
 *
 * Example: "ls -la file.txt" -> ["ls", "-la", "file.txt", NULL]
 */
char	**build_argv(t_command *command)
{
	char		**argv;
	int			size;
	int			i;
	t_list		*node;
	t_suffix	*suffix;

	if (!command)
		return (NULL);
	size = count_argv_size(command);
	argv = ft_calloc(size + 1, sizeof(char *));
	if (!argv)
		return (NULL);
	i = 0;
	if (command->cmd_word)
	{
		argv[i] = ft_strdup(command->cmd_word);
		if (!argv[i])
			return (NULL);
		i++;
	}
	node = command->cmd_suffix;
	while (node)
	{
		suffix = (t_suffix *)node->content;
		if (suffix && suffix->word)
		{
			argv[i] = ft_strdup(suffix->word);
			if (!argv[i])
			{
				while (--i >= 0)
					free(argv[i]);
				free(argv);
				return (NULL);
			}
			i++;
		}
		node = node->next;
	}
	argv[i] = NULL;
	logger_argv(argv, "build_argv result");
	return (argv);
}
