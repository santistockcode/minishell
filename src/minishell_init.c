/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell_init.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mario <mario@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/07 23:18:17 by mario             #+#    #+#             */
/*   Updated: 2026/01/13 16:46:33 by mario            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/minishell.h"


/* void	print_env(char **envp)
{
	int	i = 0;

	while (envp[i])
	{
		printf("\033[0;36m%s\033[0m\n", envp[i]);
		i++;
	}
}


void	print_env_list(t_list *env)
{
	t_env	*var;

	while (env)
	{
		var = (t_env *)env->content;
		if (var->value)
			printf("%s=%s\n", var->key, var->value);
		else
			printf("%s=\n", var->key);
		env = env->next;
	}
} */
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
t_list *init_envp(char **envp)
{
	t_list *env;
	int i;
	char * aux_key;
	char * aux;
	char * aux_value;
	
	i = 0;
	env = NULL;
	while(envp[i])
	{
		aux = ft_strchr(envp[i], '=');
		aux_key = ft_substr(envp[i], 0, aux - envp[i]);
		aux_value = ft_strdup(aux + 1);
		ft_lstadd_back(&env,ft_lstnew((void *)new_value_env(aux_key,aux_value)));
		free(aux_key);
		free(aux_value);
		i++;
	}
	return(env);
}
int init_minishell(t_shell **minishell,char **envp)
{
	*minishell = ft_calloc(sizeof(t_shell),1);
	if(!minishell)
		return (MALLOC_ERROR);
	(*minishell)->env = init_envp(envp);
	//print_env(envp);
	//print_env_list((*minishell)->env);
	return(1);
}