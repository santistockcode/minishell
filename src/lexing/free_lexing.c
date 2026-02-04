/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free_lexing.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/18 19:28:12 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/04 18:54:51 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

void	free_lexing(t_lexing *lexing)
{
	if (lexing->tokens != NULL)
		free_tokens_list(lexing->tokens);
	if (lexing->buff != NULL)
		free(lexing->buff);
	if (lexing != NULL)
		free(lexing);
}

void	free_tokens_list(t_list *tokens)
{
	t_list	*current;
	t_list	*next_node;
	t_token	*token;

	current = tokens;
	while (current)
	{
		next_node = current->next;
		token = (t_token *)current->content;
		if (token)
		{
			if (token->value)
				free(token->value);
			free(token);
		}
		free(current);
		current = next_node;
	}
}
