/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 15:46:33 by mario             #+#    #+#             */
/*   Updated: 2026/02/06 12:22:34 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

t_env	*env_get(t_list *env, char *key)
{
	t_env	*var;

	while (env)
	{
		var = (t_env *)env->content;
		if (ft_strncmp(var->key, key, ft_strlen(var->key)) == 0)
			return (var);
		env = env->next;
	}
	return (NULL);
}

void	env_set(t_list **env, char *var)
{
	t_env	*node;
	t_env	*aux;
	t_list	*new;

	node = init_node(var);
	if (!node)
		return ;
	aux = env_get(*env, node->key);
	if (aux)
	{
		free(aux->value);
		aux->value = node->value;
		free(node->key);
		free(node);
		return ;
	}
	new = ft_lstnew(node);
	if (!new)
	{
		free_env(node);
		return ;
	}
	ft_lstadd_back(env, new);
}
