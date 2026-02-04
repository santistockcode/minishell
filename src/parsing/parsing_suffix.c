/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_suffix.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 20:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/04 18:58:52 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

/**
 * parse_suffix() - Parse command suffix (redirections and arguments)
 * @suffix_list: Output list of suffixes
 * @tokens: Token list
 * @index: Current position
 *
 * Parses: ( WORD | IO_REDIRECT )*
 * Creates a list of t_suffix structures
 */
int	parse_suffix(t_list **suffix_list, t_list *tokens, int *index)
{
	t_token		*token;
	t_suffix	*suffix;
	t_list		*new_node;

	token = get_token_at(tokens, *index);
	if (!token)
		return (SUCCESS);
	
	if (!is_io_redirect(token->type) && token->type != TOKEN_WORD
		&& token->type != TOKEN_ASSIGN_WORD)
		return (SUCCESS);
	
	if (init_suffix(&suffix) != SUCCESS)
		return (MALLOC_ERROR);
	
	if (is_io_redirect(token->type))
	{
		if (parse_io_redirect_suffix(&suffix, tokens, index,
			token->type) != SUCCESS)
		{
			free(suffix);
			return (INPUT_ERROR);
		}
	}
	else if (token->type == TOKEN_WORD || token->type == TOKEN_ASSIGN_WORD)
	{
		suffix->word = ft_strdup(token->value);
		if (!suffix->word)
		{
			free(suffix);
			return (MALLOC_ERROR);
		}
		consume_token(index);
	}
	
	new_node = ft_lstnew(suffix);
	if (!new_node)
	{
		free(suffix->word);
		free(suffix);
		return (MALLOC_ERROR);
	}
	ft_lstadd_back(suffix_list, new_node);
	if (parse_suffix(suffix_list, tokens, index) != SUCCESS)
		return (INPUT_ERROR);
	
	return (SUCCESS);
}

/**
 * parse_io_redirect_suffix() - Parse IO redirection in suffix
 * @suffix: Suffix structure to fill
 * @tokens: Token list
 * @index: Current position
 * @token_type: Type of redirection token
 */
int	parse_io_redirect_suffix(t_suffix **suffix, t_list *tokens,
	int *index, t_token_type token_type)
{
	(*suffix)->io_redirect = ft_calloc(sizeof(t_io_redirect), 1);
	if (!(*suffix)->io_redirect)
		return (MALLOC_ERROR);
	
	if (token_type >= TOKEN_REDIR_IN && token_type <= TOKEN_REDIR_APPEND)
	{
		if (parse_io_file((*suffix)->io_redirect, tokens, index,
			token_type) != SUCCESS)
			return (INPUT_ERROR);
	}
	else if (token_type == TOKEN_HEREDOC)
	{
		if (parse_io_here((*suffix)->io_redirect, tokens, index) != SUCCESS)
			return (INPUT_ERROR);
	}
	
	return (SUCCESS);
}
