/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   envp_init.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mario <mario@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 17:34:26 by mario             #+#    #+#             */
/*   Updated: 2026/01/13 17:39:41 by mario            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"

t_env *new_value_env(char *str_key, char*str_value)
{
	t_env *token;

	token = malloc(sizeof(t_env));
	if(!token)
		return(NULL);//mirar posible fallo en cascada y reconduc
	token->key = ft_calloc(ft_strlen(str_key) + 1, sizeof(char));
	token->key = ft_strdup(str_key);
	token->value = ft_strdup(str_value);
	
	return(token);
}

void update_shlvl(char *value)
{
	int lvl;

	lvl = 0;
	lvl = ft_atoi(value);
	free(value);
	value = ft_itoa(lvl + 1);
}

t_list *init_envp(char **envp)
{
	t_list *env;
	int i;
	char * aux;
	char * aux_key;
	char * aux_value;
	
	i = 0;
	env = NULL;
	while(envp[i])
	{
		aux = ft_strchr(envp[i], '=');
		aux_key = ft_substr(envp[i], 0, aux - envp[i]);
		aux_value = ft_strdup(aux + 1);
		if(ft_strncmp(aux_key,"SHLVL",5) == 0)
			update_shlvl(aux_value);
		ft_lstadd_back(&env,ft_lstnew((void *)new_value_env(aux_key,aux_value)));
		free(aux_key);
		free(aux_value);
		i++;
	}
	return(env);
}
