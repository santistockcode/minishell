/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free_lexing.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/18 19:28:12 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/01 23:27:46 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

void	free_tokens_list(t_list *tokens)
{
	t_list	*current;
	t_token	*token;

	current = tokens;
	while (current)
	{
		token = (t_token *)current->content;
		if (token)
		{
			if (token->value)
				free(token->value);
			free(token);
		}
		current = current->next;
	}
	ft_lstclear(&tokens, NULL);
}
