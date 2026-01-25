/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   unset.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 15:50:36 by mario             #+#    #+#             */
/*   Updated: 2026/01/25 18:08:56 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

void delete_value(t_list *aux, t_list **prev, t_list **env)
{
	t_env *var;
	var = (t_env *)(aux)->content;
	if(*prev)
		(*prev)->next = (aux)->next;
	else
		(*env) = (aux)->next;
	free_env(var);
	free(aux);
}

// FIXME: "unset", "VAR1", "VAR3" should unset BOTH variables
void	env_unset(t_list **env, char *key)
{
	t_list *aux;
	t_list *prev;
	t_env *var;
	
	aux = *env;
	prev = NULL;
	while(aux)
	{
		var = (t_env *)aux->content;
		if(ft_strncmp(var->key,key,ft_strlen(var->key)) == 0)
		{
			delete_value(aux, &prev,env);
			return;
		}
		prev = aux;
		aux = aux->next;
	}
}
