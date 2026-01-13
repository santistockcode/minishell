/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   envp_init.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnieto-m <mnieto-m@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 17:34:26 by mario             #+#    #+#             */
/*   Updated: 2026/01/13 21:25:09 by mnieto-m         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

int free_list(t_list *env, t_env *aux)
{
	free_aux(aux);
	if(env != NULL)
		ft_lstclear(&env,free);
	return(NULL);
}
int free_aux(t_env *aux)
{
	if (!aux)
		return;
	free(aux->key);
	free(aux->value);
	free(aux);
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
	free(env->value);
	env->value = new;
}
t_env *init_node(char *envp)
{
	t_env	*env;
	char	*eq;

	eq = ft_strchr(envp, '=');
	if (!eq)
		return NULL;

	env = malloc(sizeof(*env));
	if (!env)
		return NULL;

	env->key = ft_substr(envp, 0, eq - envp);
	if (!env->key)
		return (free(env), NULL);

	env->value = ft_strdup(eq + 1);
	if (!env->value)
		return (free(env->key), free(env), NULL);

	return env;
}

t_list *init_envp(char **envp)
{
	t_list *env;
	t_env *aux;
	int i;
	
	i = 0;
	env = NULL;
	aux = malloc(sizeof(*aux));
	while(envp[i++])
	{
		aux = init_node(envp[i]);
		if(!aux)
			free_aux(aux);
		if(ft_strncmp(aux->key,"SHLVL",5) == 0)
			update_shlvl(aux->value);
		ft_lstadd_back(&env,ft_lstnew((void *)new_value_env(aux->key,aux->value)));
		if(!env)
			free_list(env, aux);// esto no es del todo correcto no se libera aux keu aux value and etc
		free_aux(aux);// modificar en otra funcion
	}
	return(env);
}
