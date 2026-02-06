/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: saalarco <saalarco@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 15:46:33 by mario             #+#    #+#             */
/*   Updated: 2026/02/06 10:05:49 by saalarco         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

void	putstr_fd_err(int n, ...);

/*
 * Get an environment variable by key.
 */
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

/*
 * Set an environment variable.
 */
int	env_set(t_list **env, char *var)
{
	t_env	*node;
	t_env	*aux;
	t_list	*new;

	node = init_node(var);
	if (!node)
		return (1);
	aux = env_get(*env, node->key);
	if (aux)
	{
		free(aux->value);
		aux->value = node->value;
		free(node->key);
		free(node);
		return (0);
	}
	new = ft_lstnew(node);
	if (!new)
	{
		free_env(node);
		return (1);
	}
	ft_lstadd_back(env, new);
	return (0);
}

// wrap env_set to return 0 on non valid input and print errors
int	export(t_list **env, char *var)
{
	int	result;

	if (ft_strchr(var, '=') == NULL || ft_strlen(var) == 0)
		return (0);
	result = env_set(env, var);
	if (result == 1)
	{
		putstr_fd_err(1, "Export: Error: Memory allocation failed\n");
		return (1);
	}
	return (0);
}
