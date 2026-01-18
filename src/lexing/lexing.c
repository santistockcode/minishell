/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexing.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/18 17:18:53 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/01/18 20:16:09 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

int	get_tokens(t_term_token *term_token)
{
	int		token_id;
	char	*term_copy;
	char	*term_i;
	t_token	*token_eof;

	if (init_lexing_utils(term_token, &term_copy, &term_i, &token_id) != 0)
		return (MALLOC_ERROR);
	while (*term_i)
	{
		if (ft_isspace(*term_i))
		{
			term_i++;
			continue ;
		}
		if (add_token(term_token, token_id, &term_i) != SUCCESS)
		{
			free(term_copy);
			return (INPUT_ERROR);
		}
		token_id++;
	}
	free(term_copy);
	init_new_token(&token_eof, token_id);
	ft_vector_push(term_token->token_array, &token_eof);
	return (SUCCESS);
}


int	add_token(t_term_token *term_token, int token_id, char **term_i)
{
	t_token	*new_token;

	new_token = NULL;
	if (init_new_token(&new_token, token_id) != SUCCESS)
		return (MALLOC_ERROR);
	if (token_switch(new_token, term_i) != SUCCESS)
		return (INPUT_ERROR);
	ft_vector_push(term_token->token_array, &new_token);
	return (SUCCESS);
}

int	token_switch(t_token *new_token, char **term_i)
{
	if (ft_strchr("<>", **term_i))
		assign_redir_token(term_i, &new_token);
	else if (ft_strchr("|", **term_i))
		assign_pipevar_token(term_i, &new_token);
	else if (ft_isprint(**term_i))
		assign_word_token(term_i, &new_token);
	if (!new_token->value)
		return (MALLOC_ERROR);
	return (SUCCESS);
}

int	reval_assign_token(t_term_token *term_token)
{
	t_token	*token;
	t_token	*prev;
	size_t	i;

	i = 0;
	while (i < term_token->token_array->size)
	{
		if (i == 0)
		{
			token = *(t_token **)ft_vector_get(term_token->token_array, i);
			if (token->type == TOKEN_WORD)
				assign_var_token(token);
		}
		else
		{
			token = *(t_token **)ft_vector_get(term_token->token_array, i);
			prev = *(t_token **)ft_vector_get(term_token->token_array, i - 1);
			if (prev->type != TOKEN_HEREDOC && token->type == TOKEN_WORD)
				assign_var_token(token);
		}
		i++;
	}
	return (SUCCESS);
}

int lexing(t_shell	* minishell)
{
	if(!get_tokens(minishell->term_token))
		return(INPUT_ERROR);
	if(!syntax_quotes(minishell->term_token))
		return(INPUT_ERROR);
	if (reval_assign_token(minishell->term_token) != SUCCESS)
		return (INPUT_ERROR);
	return(SUCCESS);
}
