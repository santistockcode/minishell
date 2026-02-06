/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing_prefix.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 20:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/06 11:55:29 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"
#include "../../include/parsing.h"

/**
 * parse_prefix_item() - Parse a single prefix token (assignment or redirection)
 * @prefix: Prefix structure to fill
 * @tokens: Token list
 * @index: Current position
 * @token_type: Type of token to parse
 *
 * Handles parsing of individual ASSIGN_WORD or IO_REDIRECT tokens
 * into the prefix structure
 */
static int	parse_prefix_item(t_prefix *prefix, t_list *tokens, int *index,
		t_token_type token_type)
{
	t_token	*token;

	token = get_token_at(tokens, *index);
	if (is_io_redirect(token_type))
	{
		if (parse_io_redirect_prefix(&prefix, tokens, index,
				token_type) != SUCCESS)
			return (INPUT_ERROR);
	}
	else if (token_type == TOKEN_ASSIGN_WORD)
	{
		prefix->assignment_word = ft_strdup(token->value);
		if (!prefix->assignment_word)
			return (MALLOC_ERROR);
		consume_token(index);
	}
	return (SUCCESS);
}

/**
 * parse_prefix() - Parse command prefix (redirections and assignments)
 * @prefix_list: Output list of prefixes
 * @tokens: Token list
 * @index: Current position
 *
 * Parses: ( ASSIGN_WORD | IO_REDIRECT )*
 * Creates a list of t_prefix structures recursively
 */
int	parse_prefix(t_list **prefix_list, t_list *tokens, int *index)
{
	t_token		*token;
	t_prefix	*prefix;
	t_list		*new_node;

	token = get_token_at(tokens, *index);
	if (!token)
		return (SUCCESS);
	if (!is_io_redirect(token->type) && token->type != TOKEN_ASSIGN_WORD)
		return (SUCCESS);
	if (init_prefix(&prefix) != SUCCESS)
		return (MALLOC_ERROR);
	if (parse_prefix_item(prefix, tokens, index, token->type) != SUCCESS)
		return (free(prefix), INPUT_ERROR);
	new_node = ft_lstnew(prefix);
	if (!new_node)
		return (free(prefix->assignment_word), free(prefix), MALLOC_ERROR);
	ft_lstadd_back(prefix_list, new_node);
	if (parse_prefix(prefix_list, tokens, index) != SUCCESS)
		return (INPUT_ERROR);
	return (SUCCESS);
}

/**
 * parse_io_redirect_prefix() - Parse IO redirection in prefix
 * @prefix: Prefix structure to fill
 * @tokens: Token list
 * @index: Current position
 * @token_type: Type of redirection token
 */
int	parse_io_redirect_prefix(t_prefix **prefix, t_list *tokens, int *index,
		t_token_type token_type)
{
	(*prefix)->io_redirect = ft_calloc(sizeof(t_io_redirect), 1);
	if (!(*prefix)->io_redirect)
		return (MALLOC_ERROR);
	if (token_type >= TOKEN_REDIR_IN && token_type <= TOKEN_REDIR_APPEND)
	{
		if (parse_io_file((*prefix)->io_redirect, tokens, index,
				token_type) != SUCCESS)
			return (INPUT_ERROR);
	}
	else if (token_type == TOKEN_HEREDOC)
	{
		if (parse_io_here((*prefix)->io_redirect, tokens, index) != SUCCESS)
			return (INPUT_ERROR);
	}
	return (SUCCESS);
}

/**
 * parse_io_file() - Parse file redirection (< > >>)
 * @io_redir: IO redirect structure to fill
 * @tokens: Token list
 * @index: Current position
 * @token_type: Type of redirection
 */
int	parse_io_file(t_io_redirect *io_redir, t_list *tokens, int *index,
		t_token_type token_type)
{
	t_token		*token;
	t_io_file	*io_file;

	io_file = ft_calloc(sizeof(t_io_file), 1);
	if (!io_file)
		return (MALLOC_ERROR);
	if (token_type == TOKEN_REDIR_IN)
		io_file->type = REDIR_IN;
	else if (token_type == TOKEN_REDIR_OUT)
		io_file->type = REDIR_OUT;
	else if (token_type == TOKEN_REDIR_APPEND)
		io_file->type = REDIR_APPEND;
	else
		return (free(io_file), INPUT_ERROR);
	consume_token(index);
	token = get_token_at(tokens, *index);
	if (!token || (token->type != TOKEN_WORD
			&& token->type != TOKEN_ASSIGN_WORD))
		return (free(io_file), INPUT_ERROR);
	io_file->filename = ft_strdup(token->value);
	if (!io_file->filename)
		return (free(io_file), MALLOC_ERROR);
	io_redir->io_file = io_file;
	consume_token(index);
	return (SUCCESS);
}

/**
 * parse_io_here() - Parse heredoc redirection (<<)
 * @io_redir: IO redirect structure to fill
 * @tokens: Token list
 * @index: Current position
 */
int	parse_io_here(t_io_redirect *io_redir, t_list *tokens, int *index)
{
	t_token		*token;
	t_io_here	*io_here;

	io_here = ft_calloc(sizeof(t_io_here), 1);
	if (!io_here)
		return (MALLOC_ERROR);
	consume_token(index);
	token = get_token_at(tokens, *index);
	if (!token || (token->type != TOKEN_WORD
			&& token->type != TOKEN_ASSIGN_WORD))
	{
		free(io_here);
		return (INPUT_ERROR);
	}
	io_here->here_end = ft_strdup(token->value);
	if (!io_here->here_end)
	{
		free(io_here);
		return (MALLOC_ERROR);
	}
	io_redir->io_here = io_here;
	consume_token(index);
	return (SUCCESS);
}
