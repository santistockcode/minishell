/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free_suffix.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/06 11:14:47 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/07 12:42:16 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

static void	free_io_redirect(t_suffix *prefix)
{
	if (prefix->io_redirect->io_file->filename != NULL)
		free(prefix->io_redirect->io_file->filename);
	if (prefix->io_redirect->io_file != NULL)
		free(prefix->io_redirect->io_file);
	if (prefix->io_redirect->io_here->filename != NULL)
		free(prefix->io_redirect->io_here->filename);
	if (prefix->io_redirect->io_here->here_end != NULL)
		free(prefix->io_redirect->io_here->here_end);
	if (prefix->io_redirect->io_here != NULL)
		free(prefix->io_redirect->io_here);
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
					free_io_redirect(suffix);
				if (suffix->io_redirect->io_here)
					free_io_redirect(suffix);
				free(suffix->io_redirect);
			}
			free(suffix);
		}
		current = current->next;
	}
	//free(suffix_list);
	// ft_lstclear(&suffix_list, free);
}

/**
 * is_io_redirect() - Check if token is an IO redirection
 * @token_type: Type to check
 *
 * Returns token_type if it's a redirection (< > >> <<), 0 otherwise
 */
int	is_io_redirect(t_token_type token_type)
{
	if (token_type >= TOKEN_REDIR_IN && token_type <= TOKEN_HEREDOC)
		return (token_type);
	return (0);
}

/**
 * consume_token() - Move to next token
 * @index: Current position pointer
 */
void	consume_token(int *index)
{
	if (index)
		(*index)++;
}
