/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexing.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/29 23:00:00 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/07 23:18:52 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

int	not_tokens(t_shell *minishell)
{
	t_token	*token;
	t_list	*first_node;

	if (!minishell || !minishell->lexing || !minishell->lexing->tokens)
	{
		if (minishell->lexing)
			free_lexing(minishell->lexing);
		minishell->lexing = NULL;
		return(1);
	}
	first_node = minishell->lexing->tokens;
	token = (t_token *)first_node->content;
	if (!token || token->type == TOKEN_EOF)
	{
		if (minishell->lexing)
			free_lexing(minishell->lexing);
		minishell->lexing = NULL;
		return(1);
	}
	return (0);
}

int	get_tokens_list(t_lexing *lexing)
{
	t_token	*token_eof;

	lexing->current = lexing->buff;
	lexing->token_id = 0;
	lexing->tokens = NULL;
	while (*(lexing->current))
	{
		if (ft_isspace(*(lexing->current)))
		{
			(lexing->current)++;
			continue ;
		}
		if (add_token_list(lexing, &(lexing->current)) != SUCCESS)
			return (MALLOC_ERROR);
		(lexing->token_id)++;
	}
	token_eof = NULL;
	if (init_new_token(&token_eof, lexing->token_id) != SUCCESS)
		return (MALLOC_ERROR);
	ft_lstadd_back(&(lexing->tokens), ft_lstnew(token_eof));
	return (SUCCESS);
}

int	add_token_list(t_lexing *lexing, char **current)
{
	t_token	*new_token;
	t_list	*new_node;

	new_token = NULL;
	if (init_new_token(&new_token, lexing->token_id) != SUCCESS)
		return (MALLOC_ERROR);
	if (token_switch(new_token, current) != SUCCESS)
	{
		if (new_token->value)
			free(new_token->value);
		free(new_token);
		return (INPUT_ERROR);
	}
	new_node = ft_lstnew(new_token);
	if (!new_node)
	{
		free(new_token->value);
		free(new_token);
		return (MALLOC_ERROR);
	}
	ft_lstadd_back(&(lexing->tokens), new_node);
	return (SUCCESS);
}

int	token_switch(t_token *new_token, char **current)
{
	if (!new_token)
		return (MALLOC_ERROR);
	if (ft_strchr("<>", **current))
		assign_redir_token(current, &new_token);
	else if (ft_strchr("|", **current))
		assign_pipevar_token(current, &new_token);
	else if (msh_isprint(**current))
		assign_word_token(current, &new_token);
	if (!new_token->value)
		return (MALLOC_ERROR);
	return (SUCCESS);
}

int	lexing(t_shell *minishell)
{
	t_lexing	*lexing_ctx;

	if (!minishell)
		return (INPUT_ERROR);
	lexing_ctx = ft_calloc(sizeof(t_lexing), 1);
	if (!lexing_ctx)
		return (MALLOC_ERROR);
	lexing_ctx->buff = readline("./minishell ");
	if (!(lexing_ctx->buff))
		return (free_lexing(lexing_ctx), EOF);
	lexing_ctx->current = NULL;
	lexing_ctx->token_id = 0;
	lexing_ctx->tokens = NULL;
	if (get_tokens_list(lexing_ctx) != SUCCESS)
		return (free_lexing(lexing_ctx), MALLOC_ERROR);
	if (syntax_quotes(lexing_ctx->tokens) != SUCCESS)
		return (free_lexing(lexing_ctx), INPUT_ERROR);
	if (lexing_ctx->tokens)
		reval_assign_token(lexing_ctx->tokens);
	minishell->lexing = lexing_ctx;
	return (SUCCESS);
}
