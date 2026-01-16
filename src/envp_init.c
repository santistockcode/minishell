/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   envp_init.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 17:34:26 by mario             #+#    #+#             */
/*   Updated: 2026/01/14 21:59:03 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

void free_aux(t_env *aux)
{
	if (!aux)
		return;
	free(aux->key);
	free(aux->value);
	free(aux);
}
void free_list(t_list **env)
{
	t_list *tmp;

	while (*env)
	{
		tmp = (*env)->next;
		free_aux((*env)->content);
		free(*env);
		*env = tmp;
	}
}
t_env *new_value_env(char *str_key, char*str_value)
{
	t_env *env;

	env = malloc(sizeof(*env));
	if (!env)
		return NULL;
	env->key = ft_strdup(str_key);
	if (!env->key)
		return (free(env), NULL);
	env->value = ft_strdup(str_value);
	if (!env->value)
		return (free(env->key), free(env), NULL);

	return (env);
}

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
t_env *init_node(char *envp)
{
	t_env	*env;
	char	*aux;

	env = malloc(sizeof(*env));
	if (!env)
		return NULL;
	aux = ft_strchr(envp, '=');
	env->key = ft_substr(envp, 0, aux - envp);
	if (!env->key)
		return (free(env), NULL);
	env->value = ft_strdup(aux + 1);
	if (!env->value)
		return (free(env->key), free(env), NULL);
	return (env);
}

t_list *init_envp(char **envp)
{
	t_env *aux;
	t_list *env;
	t_list	*node;
	int i;
	
	i = -1;
	env = NULL;
	while(envp[++i])
	{
		aux = init_node(envp[i]);
		if(!aux)
			return (free_list(&env), NULL);
		if(ft_strncmp(aux->key,"SHLVL",5) == 0)
			update_shlvl(aux);
		node = ft_lstnew(aux);
		if (!node)
			return (free_aux(aux), free_list(&env), NULL);
		ft_lstadd_back(&env,node);
	}
	return(env);
}
