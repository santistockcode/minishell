/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_init.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 17:34:26 by mario             #+#    #+#             */
/*   Updated: 2026/01/16 13:22:49 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/minishell.h"

void	update_shlvl(t_env *env)
{
	int	lvl;
	char *new;

	lvl = ft_atoi(env->value);
	new = ft_itoa(lvl + 1);
	if(!new)
		return;
	free(env->value);
	env->value = new;
}

t_env *init_node(char *str)
{
	t_env	*env;
	char	*aux;

	env = malloc(sizeof(*env));
	if (!env)
		return NULL;
	aux = ft_strchr(str, '=');
	env->key = ft_substr(str, 0, aux - str);
	if (!env->key)
		return (free_env(env), NULL);
	env->value = ft_strdup(aux + 1);
	if (!env->value)
		return (free_env(env), NULL);
	return (env);
}

t_list *init_envp(char **envp)
{
	t_env *node;
	t_list *env;
	t_list	*aux;
	int i;
	
	i = -1;
	env = NULL;
	while(envp[++i])
	{
		node = init_node(envp[i]);
		if(!node)
			return (free_list(&env), NULL);
		if(ft_strncmp(node->key,"SHLVL",6) == 0)
			update_shlvl(node);
		aux = ft_lstnew(node);
		if (!aux)
			return (free_env(node), free_list(&env), NULL);
		ft_lstadd_back(&env,aux);
	}
	return(env);
}
