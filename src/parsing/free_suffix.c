/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free_suffix.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/06 11:14:47 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/07 23:15:00 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

static void	free_io_redirect(t_suffix *suffix)
{
	if (!suffix || !suffix->io_redirect)
		return ;
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

static void	delete_suffix(void *content)
{
	t_suffix	*suffix;

	if (!content)
		return ;
	suffix = (t_suffix *)content;
	free(suffix->word);
	if (suffix->io_redirect)
		free_io_redirect(suffix);
	free(suffix);
}

/**
 * free_suffixes() - Free suffix list
 * @suffix_list: List of t_suffix nodes
 */
void	free_suffixes(t_list *suffix_list)
{
	ft_lstclear(&suffix_list, delete_suffix);
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
