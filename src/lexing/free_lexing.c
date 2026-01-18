/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free_lexing.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/18 19:28:12 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/01/18 19:39:49 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

void free_tokens(t_vector *token_array)
{
	size_t i;
	t_token *token;
	
	i = 0;
	while(i< token_array->size)
	{
		token = *(t_token **)ft_vector_get(token_array,i);
		free(token->value);
		free(token);
		i++;
	}
	ft_vector_free(token_array);
	free(token_array);
}