/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_lexing.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/20 22:47:55 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/01/20 22:48:28 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

char	*ft_getenv(char *value)
{
	if (!value)
		return (NULL);
	if (getenv(value))
		return (ft_strdup(getenv(value)));
	return (NULL);
}
int	init_lexing_utils(t_term_token *term_token, char **term_copy,
	char **term_i, int *token_id)
{
	*token_id = 0;
	*term_copy = ft_strdup(term_token->term_line);
	*term_i = *term_copy;
	term_token->token_array = malloc(sizeof(t_vector));
	if (!term_token->token_array)
		return (MALLOC_ERROR);
	ft_vector_init(term_token->token_array, sizeof(t_token *));
	return (SUCCESS);
}

int	init_new_token(t_token **new_token, int token_id)
{
	*new_token = ft_calloc(sizeof(t_token), 1);
	if (!*new_token)
		return (MALLOC_ERROR);
	(*new_token)->id = token_id;
	(*new_token)->type = TOKEN_EOF;
	(*new_token)->value = NULL;
	(*new_token)->syntax_error = SYNTAX_OK;
	return (SUCCESS);
}