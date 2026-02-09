/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_suffix.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 20:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/06 11:32:00 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

/**
 * parse_suffix_item() - Parse a single suffix token (word or redirection)
 * @suffix: Suffix structure to fill
 * @tokens: Token list
 * @index: Current position
 * @token_type: Type of token to parse
 *
 * Handles parsing of individual WORD or IO_REDIRECT tokens
 * into the suffix structure
 */
static int	parse_suffix_item(t_suffix *suffix, t_list *tokens, int *index,
		t_token_type token_type)
{
	t_token	*token;

	token = get_token_at(tokens, *index);
	if (is_io_redirect(token_type))
	{
		if (parse_io_redirect_suffix(&suffix, tokens, index,
				token_type) != SUCCESS)
			return (INPUT_ERROR);
	}
	else if (token_type == TOKEN_WORD || token_type == TOKEN_ASSIGN_WORD)
	{
		suffix->word = ft_strdup(token->value);
		if (!suffix->word)
			return (MALLOC_ERROR);
		consume_token(index);
	}
	return (SUCCESS);
}

/**
 * parse_suffix() - Parse command suffix (redirections and arguments)
 * @suffix_list: Output list of suffixes
 * @tokens: Token list
 * @index: Current position
 *
 * Parses: ( WORD | IO_REDIRECT )*
 * Creates a list of t_suffix structures recursively
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
	if (parse_suffix_item(suffix, tokens, index, token->type) != SUCCESS)
		return (free(suffix), INPUT_ERROR);
	new_node = ft_lstnew(suffix);
	if (!new_node)
		return (free(suffix->word), free(suffix), MALLOC_ERROR);
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
int	parse_io_redirect_suffix(t_suffix **suffix, t_list *tokens, int *index,
		t_token_type token_type)
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
