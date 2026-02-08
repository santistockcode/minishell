/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   unset.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 15:50:36 by mario             #+#    #+#             */
/*   Updated: 2026/02/06 16:44:04 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

/*
 * Delete a variable from the environment.
 */
void	delete_value(t_list *aux, t_list **prev, t_list **env)
{
	t_env	*var;

	var = (t_env *)(aux)->content;
	if (*prev)
		(*prev)->next = (aux)->next;
	else
		(*env) = (aux)->next;
	free_env(var);
	free(aux);
}

// FIXME: "unset", "VAR1", "VAR3" should unset BOTH variables
void	env_unset(t_list **env, char *key)
{
	t_list	*aux;
	t_list	*prev;
	t_env	*var;

	aux = *env;
	prev = NULL;
	while (aux)
	{
		var = (t_env *)aux->content;
		if (ft_strncmp(var->key, key, ft_strlen(var->key)) == 0)
		{
			delete_value(aux, &prev, env);
			return ;
		}
		prev = aux;
		aux = aux->next;
	}
}
