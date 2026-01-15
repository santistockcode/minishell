/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   free_env.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mario <mario@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/14 23:12:42 by mnieto-m          #+#    #+#             */
/*   Updated: 2026/01/15 18:43:59 by mario            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/env.h"

void free_env(t_env *aux)
{
	if (!aux)
		return;
	if(aux->key)
		free(aux->key);
	if(aux->value)
		free(aux->value);
	if(aux)
		free(aux);
}
void free_list(t_list **env)
{
	t_list *tmp;

	while (*env)
	{
		tmp = (*env)->next;
		free_env((*env)->content);
		free(*env);
		*env = tmp;
	}
}