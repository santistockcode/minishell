/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free_env.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 23:12:42 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/02/06 12:23:15 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

void	free_env(t_env *aux)
{
	if (!aux)
		return ;
	if (aux->key)
		free(aux->key);
	if (aux->value)
		free(aux->value);
	if (aux)
		free(aux);
}

void	free_list(t_list **env)
{
	t_list	*tmp;

	while (*env)
	{
		tmp = (*env)->next;
		free_env((*env)->content);
		free(*env);
		*env = tmp;
	}
}
