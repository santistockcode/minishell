/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   build_argv.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/06 12:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/06 23:38:13 by mnieto-m         ###   ########.fr       */
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

static int	add_cmd_word(char **argv, char *cmd_word)
{
	if (!cmd_word)
		return (0);
	argv[0] = ft_strdup(cmd_word);
	if (!argv[0])
		return (-1);
	return (1);
}

static int	add_suffix_words(char **argv, t_list *suffix_list, int start_idx)
{
	t_list		*node;
	t_suffix	*suffix;
	int			i;

	i = start_idx;
	node = suffix_list;
	while (node)
	{
		suffix = (t_suffix *)node->content;
		if (suffix && suffix->word)
		{
			argv[i] = ft_strdup(suffix->word);
			if (!argv[i])
				return (-1);
			i++;
		}
		node = node->next;
	}
	return (i);
}

char	**build_argv(t_command *command)
{
	char	**argv;
	int		size;
	int		i;

	if (!command)
		return (NULL);
	size = count_argv_size(command);
	argv = ft_calloc(size + 1, sizeof(char *));
	if (!argv)
		return (NULL);
	i = add_cmd_word(argv, command->cmd_word);
	if (i < 0)
		return (NULL);
	i = add_suffix_words(argv, command->cmd_suffix, i);
	if (i < 0)
	{
		while (--i >= 0)
			free(argv[i]);
		return (free(argv), NULL);
	}
	argv[i] = NULL;
	return (argv);
}
